#pragma once


#include <cstddef>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace elfw {

    struct Data {
        const char* ptr;
        const std::size_t size;
    };


    class ResourceLoader {

    public:
        ResourceLoader(const std::string& file) : data() {
            if (!readBinary(file, data)) {
                fprintf(stderr, "Cannot load binary: '%s'", file.c_str());
                exit(-11);
            }
            std::string metaFile = std::string(file) + ".meta";

            if (!readMeta(metaFile, meta)) {
                fprintf(stderr, "Cannot load meta: '%s'", metaFile.c_str());
                exit(-11);
            }

            for (auto v : meta) {
                printf("[Resources] '%s' : %zd %zd\n", v.first.c_str(), v.second.first, v.second.second);
            }

        }
        ~ResourceLoader() {}

        Data get(const std::string& path) {
            if (meta.count(path) == 0) {
                fprintf(stderr, "Cannot find resource with path: '%s'", path.c_str());
                exit(-12);
            }

            auto m = meta[path];
            return { &data[m.first], m.second };
        }

    private:
        using MetaMap = std::unordered_map<std::string, std::pair<size_t, size_t>>;

        static bool readBinary(const std::string& file, std::vector<char>& data) {
            std::ifstream f(file, std::ios::in);
            if (!f.good()) {
                return false;
            }

            f.seekg(0, f.end);
            auto s = (size_t)f.tellg();
            f.seekg(0, f.beg);
            data.resize(s);
            f.read(data.data(), data.size());

            printf("[Resource] Read %zd bytes from binary '%s'\n", data.size(), file.c_str());

            return true;
        }


        static bool readMeta(const std::string& metaFile, MetaMap& metadata) {
            std::ifstream f(metaFile, std::ios::in);
            if (!f.good()) {
                return false;
            }

            while (!f.eof()) {
                std::string filename;
                std::size_t offset, size;

                auto nextChar = f.peek();
                f >> filename;
                f >> offset;
                f >> size;

                if (filename.size() != 0) {
                    metadata.insert({filename, std::make_pair(offset, size)});
                }

            }

            return true;
        }

        std::vector<char> data;
        MetaMap meta;


    };


}
