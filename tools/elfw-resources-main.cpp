//
// Created by Miles Gibson on 20/02/17.
//


#include <cstdio>
#include <vector>
#include <map>
#include <fstream>

#if _WIN32
#define getcwd _getcwd
#else
#include <unistd.h>
#include <sstream>

#endif // _WIN32

#include "../mkzbase/slice.hpp"

#include "elfw-resources.h"

namespace {


    struct Data {
        std::vector<char> binary;
        std::map< std::string, std::pair<size_t, size_t>> buffers;
    };


    template <typename Seq>
    Data fileListToData(Seq&& files) {
        Data d = {};

        for (const auto& file : files) {
            std::ifstream f(file, std::ios::in);

            // get length of file:
            f.seekg (0, f.end);
            std::streamsize size = f.tellg();
            f.seekg (0, f.beg);


            printf("['%s' -> size=%ld]\n", file.c_str(), size);
            if (!f.is_open()) {
                fprintf(stderr, "Cannot find file: %s\n", file.c_str());
                exit(-10);
            }

            size_t startIdx = d.binary.size();
            d.binary.resize( d.binary.size() + size + 1 );
            if (f.read(&d.binary[startIdx], size)) {
                d.binary[startIdx + size ] = '\0';
                d.buffers[file] = std::make_pair(startIdx, (size_t)size);
            }

        }
        return d;
    }


    template <typename F>
    std::vector<char> stringBuffer(F&& f) {
        std::stringstream o;
        f(o);

        auto s = o.str();
        return std::vector<char>(s.begin(), s.end());
    }

    template <typename F>
    std::vector<char> withEachBufferMeta(const Data& data, F&& f) {
        return stringBuffer([&](auto& o){
            for (auto it = data.buffers.begin(); it != data.buffers.end(); ++it) {
                f(o, it->first, it->second.first, it->second.second);
            }
        });
    }

    std::vector<char> metaToBinary(const Data& data) {
        return withEachBufferMeta(data, [&](auto& o, std::string name, size_t offset, size_t size) {
            o << name << "\t" << offset << "\t" << size << "\n";
        });
    }

//    std::vector<char> metaToHeader(const Data& data) {
//        static const auto isValidChar = [](const auto c){
//              return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-';
//        };
//
//        const auto names = "";
//        return stringBuffer([&](auto& o){
//            o << "enum {\n"
//            for (auto it = data.buffers.begin(); it != data.buffers.end(); ++it) {
//
//                auto f = std::string(it->first.c_str());
//                std::replace_if( f.begin(), f.end(), isValidChar, '_');
//                o << "  " << f << ",\n";
//            }
//            o << "}; // enum\n"
//        });
//    }

    // Help
    void usage(const char* argv0) {
        printf("USAGE:\n\n\t%s OUTPUT_FILE_NAME INPUT_FILES...\n", argv0);
    }



    // we failed :)
    template <typename... T>
    void fail(const char* format, T&&... what) {
        fprintf(stderr, format, what...);
        fprintf(stderr, "\n");
        exit(-10);
    }


    template <typename Seq, typename Fn>
    auto mapv(Seq&& seq, Fn&& f) -> std::vector<decltype(f(seq[0]))> {
        using std::begin;
        using std::end;
        using T = decltype(f(seq[0]));
        std::vector<T> o = {};
        for (const auto& val : seq) { o.emplace_back(f(val)); }
        return o;
    }


    std::string getWorkingDirectory() {
        const size_t chunkSize=255;
        const int maxChunks=10240; // 2550 KiBs of current path are more than enough

        char stackBuffer[chunkSize]; // Stack buffer for the "normal" case
        if(getcwd(stackBuffer,sizeof(stackBuffer))!=NULL)
            return stackBuffer;
        if(errno!=ERANGE)
        {
            // It's not ERANGE, so we don't know how to handle it
            throw std::runtime_error("Cannot determine the current path.");
            // Of course you may choose a different error reporting method
        }
        // Ok, the stack buffer isn't long enough; fallback to heap allocation
        for(int chunks=2; chunks<maxChunks ; chunks++)
        {
            // With boost use scoped_ptr; in C++0x, use unique_ptr
            // If you want to be less C++ but more efficient you may want to use realloc
            std::auto_ptr<char> cwd(new char[chunkSize*chunks]);
            if(getcwd(cwd.get(),chunkSize*chunks)!=NULL)
                return cwd.get();
            if(errno!=ERANGE)
            {
                // It's not ERANGE, so we don't know how to handle it
                throw std::runtime_error("Cannot determine the current path.");
                // Of course you may choose a different error reporting method
            }
        }
        throw std::runtime_error("Cannot determine the current path; the path is apparently unreasonably long");
    }


    bool writeFile(const std::string& fn, const std::vector<char>& data) {
        std::ofstream binary(fn, std::ios::out);
        if (!binary.good()) {
            fprintf(stderr, "[Error while writing '%s']", fn.c_str());
            return false;
        }

        binary.write(data.data(), data.size());
        printf("[Written] %s\n", fn.c_str());
        return true;
    }
}



int main(const int argc, const char** argv) {
    const auto cwd = getWorkingDirectory();
    printf("[Inside %s]\n", cwd.c_str() );



    if (argc < 3) {
        usage(argv[0]);
        fail("Not enough arguments (got [%d])", argc - 1);
    }

    const auto dataFile = std::string(argv[1]);

    const auto data = fileListToData(
            mapv(
                    mkz::make_slice(&argv[2], (size_t)(argc - 2)),
                    [](const char* s) { return std::string(s); }
            )
    );

    puts("===========\n");

    const auto metadata = metaToBinary(data);
    if (!writeFile(dataFile, data.binary)
        || !writeFile(dataFile + ".meta", metadata))
    {
        exit(-11);
    }

    puts("== Packing done, checking load \n");

    // test
    elfw::ResourceLoader l(dataFile);


}
