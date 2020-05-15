#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

#include "utils/execution_timer.h"

namespace evo
{
    namespace log
    {
        namespace midx = boost::multi_index;
        namespace fs = boost::filesystem;

        class TimeLogger
        {
          public:
            TimeLogger(fs::path _filepath);

            void addColumn(const std::string& header);

            template<typename T>
            void addValue(const std::string& header, const T& value)
            {
                auto& header_index = line_.get<1>();
                auto it = header_index.find(header);
                if (it != header_index.end())
                    header_index.modify(it, [value](line& l) { l.value = std::to_string(value); });
                else
                    throw std::runtime_error("This header was not found in the line container.");
            }

            void start();
            void tick(const std::string& chkname = std::string());
            void stop();

            void memorize();
            void writeFile(bool append);
            void write(std::ostream& stream);

          private:
            struct line
            {
                std::string header;
                std::string value;
            };

            typedef midx::multi_index_container<line,
                midx::indexed_by<midx::random_access<>,
                    midx::ordered_unique<midx::member<line, std::string, &line::header>>>>
                midx_line;

            std::string printHeaders() const;
            std::string printLine() const;

            bool isHeadersPrinted = false;
            fs::path filepath;
            utils::ExecutionTimer<> timer;
            std::vector<std::string> linesCache;
            midx_line line_;
        };

        template<>
        inline void TimeLogger::addValue(const std::string& header, const std::string& value)
        {
            auto& header_index = line_.get<1>();
            auto it = header_index.find(header);
            if (it != header_index.end())
                header_index.modify(it, [&value](line& l) { l.value = value; });
            else
                throw std::runtime_error("This header was not found in the line container.");
        }
    } // namespace log
} // namespace evo
