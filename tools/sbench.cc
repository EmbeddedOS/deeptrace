#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>

class Arg
{
    std::vector<std::string> _args;
    std::vector<const char *> _buffer;

private:
    void Push(const std::string &arg)
    {
        _args.push_back(arg);
        _buffer.push_back(_args[_args.size() - 1].c_str());
    }

    void Finish()
    {
        if (!_buffer.size())
        {
            return;
        }

        if (_buffer[_buffer.size() - 1] != nullptr)
        {
            _buffer.push_back(nullptr);
        }
    }

public:
    Arg(const std::string &arg)
    {
        Push(arg);
    }

    template <typename... Args>
    Arg(const std::string &head, Args... args)
    {
        Push(head);
        Arg(args...);
    }

    Arg &Add(const std::string &arg)
    {
        Push(arg);
        return *this;
    }

    operator const char **()
    {
        Finish();
        return _buffer.data();
    }

    ~Arg()
    {
    }
};

class Process
{
    pid_t _pid;

public:
    Process(pid_t id) : _pid{id} {}
    Process(const std::string &bin)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // Children process start from here.
        }
        else if (pid > 0)
        { // Current process continue here.
        }
        else
        { // Error case.
        }
    }
};

class Tracer
{
public:
    Tracer() = default;
    ~Tracer() = default;
};

int main(int argc, char *argv[])
{
    std::string pid{};
    std::string output_file{};

    auto usage = [](int exit_code) -> void
    {
        std::cerr << "Usage: sbench [-p pid] [-o output_file] [-h]\n"
                  << "Options:\n"
                  << "  -p pid          Specify the process ID to benchmark\n"
                  << "  -o output_file  Specify the output file for results\n"
                  << "  -h              Display this help message\n";
        exit(exit_code);
    };

    if (argc < 1)
    {
        usage(EXIT_FAILURE);
    }

    int opt = 0;
    while ((opt = getopt(argc, argv, "hp:o:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            pid = std::string{optarg};
            break;
        case 'o':
            output_file = std::string{optarg};
            break;
        case 'h':
            usage(EXIT_SUCCESS);
            break;
        default:
            usage(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; ++i)
    {
        std::cerr << "Process name " << argv[i] << "\n";
    }

    Arg args{"first", "second", "third", "fourth"};

    args.Add("Fifth").Add("Sixth").Add("Seventh");

    const char **char_args = args;

    for (int i = 0; char_args[i] != nullptr; i++)
    {
        std::cout << (char *)char_args[i] << "\n";
    }

    return 0;
}