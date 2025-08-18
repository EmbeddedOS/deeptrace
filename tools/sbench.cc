#include <unistd.h>
#include <iostream>

int main(int argc, char *argv[])
{
    std::string pid{};
    std::string output_file{};

    auto usage = [](int exit_code)
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

    return 0;
}