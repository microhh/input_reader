#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "Data_block.h"
#include "Convert.h"

namespace
{
    std::vector<std::string> split_line(std::string& line, const std::string splitter)
    {
        std::vector<std::string> strings;

        // Strip of the comments.
        boost::trim(line);
        boost::split(strings, line, boost::is_any_of("#"));

        // Keep part that is not comment and clear strings.
        if (strings.size() >= 2)
            line = strings[0];
        strings.clear();

        // Return if line is empty.
        if (line.empty())
            return strings;

        // Strip of all the whitespace.
        boost::trim(line);

        // Split string on the given splitter.
        boost::split(strings, line, boost::is_any_of(splitter), boost::token_compress_on);

        return strings;
    }

    template<typename T>
    T convert_from_string(const std::string& value)
    {
        std::istringstream ss(value);

        T item = Convert::get_item_from_stream<T>(ss);
        Convert::check_item<T>(item);

        return item;
    }
}

Data_block::Data_block(const std::string& file_name)
{
    // Read file and throw exception on error.
    std::ifstream infile;

    infile.open(file_name);
    if (!infile.good())
        throw std::runtime_error("Illegal file name");

    std::string line;

    // First, read the header.
    int number_of_vectors;
    int line_number = 0;
    std::vector<std::string> header_items;

    while (std::getline(infile, line))
    {
        ++line_number;
        std::vector<std::string> strings = split_line(line, " \t");
        if (strings.size() == 0)
            continue;

        number_of_vectors = strings.size();

        for (const std::string& s : strings)
        {
            auto it = std::find(header_items.begin(), header_items.end(), s);
            if (it != header_items.end())
                throw std::runtime_error("Duplicate name in header");
            else
                header_items.push_back(s);
        }
        break;
    }

    // Second, read the data.
    while (std::getline(infile, line))
    {
        ++line_number;
        std::vector<std::string> strings = split_line(line, " \t");
        if (strings.size() == 0)
            continue;

        // Check if the number if items match with the header.
        if (strings.size() != number_of_vectors)
        {
            std::string error_string = "Illegal number of items on line ";
            error_string += std::to_string(line_number);
            throw std::runtime_error(error_string);
        }

        // Insert data into the vectors.
        auto it_s = strings.begin();
        for (auto& h : header_items)
        {
            data_series[h].push_back(*it_s);
            ++it_s;
        }
    }
}

template <typename T>
void Data_block::get_vector(std::vector<T>& destination,
                            const std::string& name,
                            const size_t length,
                            const size_t source_start_index,
                            const size_t destination_start_index)
{
    if ( (data_series.at(name).size() < source_start_index + length) )
    {
        std::string error_string = "Out of bounds read of vector ";
        error_string += name;
        throw std::runtime_error(error_string);
    }
    if ( (destination.size() < destination_start_index + length) )
    {
        std::string error_string = "Out of bounds write of vector ";
        error_string += name;
        throw std::runtime_error(error_string);
    }

    std::transform(data_series.at(name).begin()+source_start_index,
            data_series.at(name).begin()+(source_start_index+length),
            destination.begin()+destination_start_index,
            [](const std::string value) { return convert_from_string<T>(value); });
}

template void Data_block::get_vector(std::vector<std::string>&, const std::string&, const size_t, const size_t, const size_t);
template void Data_block::get_vector(std::vector<double>&, const std::string&, const size_t, const size_t, const size_t);
template void Data_block::get_vector(std::vector<float>&, const std::string&, const size_t, const size_t, const size_t);
template void Data_block::get_vector(std::vector<int>&, const std::string&, const size_t, const size_t, const size_t);

