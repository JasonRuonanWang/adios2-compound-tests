#include <adios2.h>

#include <iostream>
#include <numeric>
#include <vector>
#include <chrono>

size_t arraySize = 25600;
size_t variables = 80;
size_t steps = 1000;

int main(int argc, char *argv[])
{

    if(argc >= 2) arraySize = std::stoi(argv[1]);
    if(argc >= 3) variables = std::stoi(argv[2]);
    if(argc >= 4) steps = std::stoi(argv[3]);

    MPI_Init(&argc, &argv);
    int worldRank, worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    {
        adios2::Dims shape({(size_t)worldSize, variables, arraySize});
        adios2::Dims start({(size_t)worldRank, 0, 0});
        adios2::Dims count({1, variables, arraySize});

        auto timerStart = std::chrono::system_clock::now();
        auto timerNow = std::chrono::system_clock::now();
        std::chrono::duration<double> duration;

        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("TestIO");

        adios2::Engine engine = io.Open("Test", adios2::Mode::Write);
        auto varFloats = io.DefineVariable<float>("varFloats", shape, start, count);

        size_t datasize = std::accumulate(count.begin(), count.end(), 1, std::multiplies<size_t>());
        std::vector<float> vecFloats(datasize);

        for(size_t step = 0; step < steps; ++step)
        {
            timerNow = std::chrono::system_clock::now();
            duration = timerNow - timerStart;
            engine.BeginStep();
            engine.Put(varFloats, vecFloats.data());
            engine.EndStep();
        }

        engine.Close();

        size_t totalDatasize = steps * std::accumulate(shape.begin(), shape.end(), sizeof(float), std::multiplies<size_t>());

        timerNow = std::chrono::system_clock::now();
        duration = timerNow - timerStart;

        MPI_Barrier(MPI_COMM_WORLD);

        if(worldRank == 0)
        {
            std::cout << "===============================================================" << std::endl;
            std::cout << "Compound Array Size " << arraySize * variables << ", Variables " << 1 << ", Writers " << worldSize << ", time " << duration.count() << " seconds, " << steps << " steps, " << "total data size " << totalDatasize / 1000000000 << " GB, data rate " <<  totalDatasize / duration.count() / 1000000000 << " GB/s" << std::endl;
            std::cout << "===============================================================" << std::endl;
        }
    }

    {
        adios2::Dims shape({(size_t)worldSize, arraySize});
        adios2::Dims start({(size_t)worldRank, 0});
        adios2::Dims count({1, arraySize});

        auto timerStart = std::chrono::system_clock::now();
        auto timerNow = std::chrono::system_clock::now();
        std::chrono::duration<double> duration;

        adios2::ADIOS adios;
        adios2::IO io = adios.DeclareIO("TestIO");

        adios2::Engine engine = io.Open("Test", adios2::Mode::Write);

        std::vector<adios2::Variable<float>> vars;

        for(int i=0; i<variables; ++i)
        {
            vars.emplace_back( io.DefineVariable<float>("varFloats"+std::to_string(i), shape, start, count));
        }

        size_t datasize = std::accumulate(count.begin(), count.end(), 1, std::multiplies<size_t>());
        std::vector<float> vecFloats(datasize);

        for(size_t step = 0; step < steps; ++step)
        {
            timerNow = std::chrono::system_clock::now();
            duration = timerNow - timerStart;
            engine.BeginStep();
            for(auto &var : vars)
            {
                engine.Put(var, vecFloats.data());
            }
            engine.EndStep();
        }

        engine.Close();

        size_t totalDatasize = steps * variables * std::accumulate(shape.begin(), shape.end(), sizeof(float), std::multiplies<size_t>());

        timerNow = std::chrono::system_clock::now();
        duration = timerNow - timerStart;

        MPI_Barrier(MPI_COMM_WORLD);

        if(worldRank == 0)
        {
            std::cout << "===============================================================" << std::endl;
            std::cout << "Basic Array Size " << arraySize << ", Variables " << variables << ", Writers " << worldSize << ", time " << duration.count() << " seconds, " << steps << " steps, " << "total data size " << totalDatasize / 1000000000 << " GB, data rate " <<  totalDatasize / duration.count() / 1000000000 << " GB/s" << std::endl;
            std::cout << "===============================================================" << std::endl;
        }
    }


    MPI_Finalize();

    return 0;
}
