#pragma once

#include "utils.hpp"

#include <fstream>
#include <ostream>

#ifdef FSEXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include "log.hpp"

namespace ioh::common::file {

    class FileSystemItem {
    protected:
        fs::path root_;
        std::string name_;
        fs::path path_;

    public:
        FileSystemItem(){}

        FileSystemItem(fs::path root, const std::string &name)
            : root_(std::move(root)),
              name_(name),
              path_(root_ / name) {
        }

        void path(const fs::path& path) {
            root_ = path.parent_path();
            name_ = (path.filename() != "" ? path.filename() : path.stem()).generic_string();
            path_ = path;
        }

        [[nodiscard]] fs::path path() const {
            return path_;
        }

        [[nodiscard]] fs::path root() const {
            return root_;
        }

        [[nodiscard]] std::string name() const {
            return name_;
        }

        void remove() const {
            remove_all(path_);
        }

        friend std::ostream& operator<<(std::ostream& os, const FileSystemItem& obj) {
            return os << "FileSystemItem: " << obj.path_;
        }              
    };

    class UniqueFolder : public FileSystemItem {
    public:
        explicit UniqueFolder(fs::path root, const std::string &name)
            : FileSystemItem(std::move(root), name) {
            if (!exists(root_))
                create_directories(root_);

            path_ = root_ / name;
            auto index = 0;
            while (exists(path_)) {
                ++index;
                name_ = name + '-' + to_string(index);
                path_ = root_ / name_;
            }
            create_directories(path_);
        }

        explicit UniqueFolder(const std::string& name)
            : UniqueFolder(fs::current_path(), name) {
        }

        explicit UniqueFolder(){}
    };

    class BufferedFileStream : public FileSystemItem {
        std::string buffer_;
        std::ofstream stream_;

    public:
        explicit BufferedFileStream(fs::path root, const std::string &name)
            : FileSystemItem(std::move(root), name),
              stream_(path_, std::ios_base::app) {
        }

        explicit BufferedFileStream(const std::string &name)
            : BufferedFileStream(fs::current_path(), name) {
        }

        explicit BufferedFileStream(const fs::path &path)
            : BufferedFileStream(path.parent_path(), path.filename().generic_string()) {
        }
  
        BufferedFileStream& operator=(BufferedFileStream &&other) noexcept {
            if (this == &other)
                return *this;

            close();
            FileSystemItem::operator =(std::move(other));
            buffer_ = std::move(other.buffer_);
            stream_ = std::move(other.stream_);
            return *this;
        }

        explicit BufferedFileStream() = default;

        void open(const fs::path &path) {
            this->close();
            this->path(path);
            stream_.open(path_, std::ios_base::app);
        }

        void close() {
            if (stream_.is_open()) {
                flush();
                stream_.close();
            }
        }

        ~BufferedFileStream() {
            close();
        }

        void write(const std::string &data, const bool on_newline = false) {
            if(stream_.is_open()) {
                if (on_newline && (stream_.tellp() != 0 || !buffer_.empty())) {
                    buffer_ += "\n" + data;
                }else {
                    buffer_ += data;
                }
                if (buffer_.size() > IOH_MAX_BUFFER_SIZE)
                    flush();
            }            
        }

        void flush() {
          stream_ << buffer_;
          buffer_.clear();
          stream_.flush();
        }

        [[nodiscard]] std::string buffer() const {
            return buffer_;
        }

        void remove() {
            stream_.close();
            fs::remove(path_);
        }

        BufferedFileStream& operator<<(const std::string& data) {
            write(data);
            return *(this);
        }

        BufferedFileStream(const BufferedFileStream &other) = delete;
        BufferedFileStream(BufferedFileStream &&other) noexcept = delete;
        BufferedFileStream & operator=(const BufferedFileStream &other) = delete;
    };


    /**
     * \brief Opens a file
     * \param filename The path of the file to be opened
     * \return a stream handle to the opened file
     */
    inline std::ifstream open_file(fs::path &filename) {
        if (!exists(filename))
            IOH_DBG(error,"Cannot find file " << filename.generic_string());

        std::ifstream file(filename);
        if (!file.is_open())
            IOH_DBG(error,"Cannot open file " << filename.generic_string());
        return file;
    }


}
