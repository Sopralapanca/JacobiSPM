#include <iostream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <fstream>

#include "./utils/utimer.cpp"
#include "./utils/utility.h"

// compile with g++ -I ../fastflow/ ./utils/utility.cpp -pthread -O3 ff_parallel.cpp  -o ff_parallel

using namespace std;


int main(int argc, char * argv[]) {
    if(argc != 6){
        cout << "Usage: matrix_size number_of_iterations number_of_threads check_flag chunksize" << endl;
        return (0);
    }

    int n = atoi(argv[1]); // matrix size
    int k = atoi(argv[2]); // number of iterations
    int nw = atoi(argv[3]);
    bool check = (atoi(argv[4]) == 0 ? false : true); // Boolean for debugging purposes
    int chunk_size = atoi(argv[5]);
    int seed = 123; //int seed = atoi(argv[3]);

    // generate a random linear system
    vector<vector<float>> matrix = GenerateRandomMatrix(n, seed);

    vector<float> actual_x(n);
    actual_x = GenerateRandomVector(n, 0.0, 1.0, seed);

    // compute the linear system to find b
    vector<float> b(n);
    b=compute_b(matrix, actual_x, n);

    // initialize a vector with all zeroes to be used to find a solution using Jacobi
    vector<float> x(n, 0.0), new_x(n, 0.0);

    // number of workers nw
    // use spinwaits instead of locks -> to use with small grain computations
    // avoid barrier at the end of parallel for
    ff::ParallelFor pf(nw, true, false);

    long u;
    {
        utimer tpar("FastFlow", &u);
        for (int it=0; it<k; it++) {
            pf.parallel_for(0, n, 1,
                             chunk_size,
                            [&](const int i){
                                float sum = 0;
                                for (int j = 0; j < n; j++) {
                                    if(j!=i){
                                        sum = matrix[i][j] * x[j];
                                    }
                                }

                                new_x[i] = (b[i] - sum) / matrix[i][i];
                            }, nw);
            x=new_x;


        }
    }

    ofstream myfile;
    myfile.open ("./results/ff_parallel.txt", std::ios_base::app);

    myfile << "Matrix size: " << n <<" iterations: "<< k << "\n";
    myfile << "Workers: " << nw << "\n";
    myfile << "Total execution time: " << u <<" usec\n";
    myfile << "Time per iteration: " << u/k <<" usec\n";
    myfile << "Time per thread: " << (u/k)/nw <<" usec\n\n";

    myfile.close();

    cout << "Time per iteration = " << u/k <<" usec"<< endl;
    cout << "Time per thread: " << (u/k)/nw <<" usec" <<endl;

    if (check){
        cout << "MATRIX A" << endl;
        print_matrix(matrix, n);

        cout << "vector b" << endl;
        print_vector(b);

        cout << "vector x" << endl;
        print_vector(x);

        //pf.ffStats(cout);
    }

    return 0;
}
