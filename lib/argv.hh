#pragma once
#include <string>
#include <vector>

namespace DeepTrace
{
    class Argv
    {
        int _len{0};
        char **_buffer{nullptr};

    public:
        class ArgvBuilder
        {
            std::vector<std::string> _args;

        private:
            void Push(const std::string &arg)
            {
                _args.push_back(arg);
            }

        public:
            ArgvBuilder() = default;
            ArgvBuilder(const std::string &arg)
            {
                Push(arg);
            }

            template <typename... Args>
            ArgvBuilder(const std::string &head, Args... args) : ArgvBuilder(args...)
            {
                Push(head);
            }

            ArgvBuilder &Add(const std::string &arg)
            {
                Push(arg);
                return *this;
            }

            operator Argv()
            {
                return Argv{*this};
            }

            ~ArgvBuilder() = default;
            friend class Argv;
        };

        operator const char **()
        {
            return (const char **)_buffer;
        }

        int Len() const
        {
            return _len;
        }

        Argv() = default;
        Argv(Argv &other) = delete;
        Argv &operator=(Argv &other) = delete;

        Argv &operator=(Argv &&other)
        {
            _len = other._len;
            _buffer = other._buffer;

            other._len = 0;
            other._buffer = nullptr;

            return *this;
        }

        Argv(Argv &&other)
        {
            _len = other._len;
            _buffer = other._buffer;

            other._len = 0;
            other._buffer = nullptr;
        }

        Argv(const ArgvBuilder &builder)
        {
            if (!builder._args.size())
            {
                return;
            }

            _len = builder._args.size() + 1;
            _buffer = new char *[_len];

            for (std::size_t i = 0; i < builder._args.size(); i++)
            {
                if (!builder._args[i].size())
                {
                    continue;
                }

                _buffer[i] = new char[builder._args[i].size()];
                builder._args[i].copy(_buffer[i], builder._args[i].size());
            }

            _buffer[_len - 1] = nullptr;
        }

        ~Argv()
        {
            if (_len == 0 || !_buffer)
            {
                return;
            }

            for (int i = 0; i < _len; i++)
            {
                delete[] _buffer[i];
            }

            delete[] _buffer;
        }
    };
}