#include <iostream>
#include <stdlib.h>
#include <vector>
#include <atomic>
#include <queue>
#include <mutex>
#include <functional>
#include <algorithm>
#include <barrier>
#include <fstream>
#include <sstream>

#include "./utils/utimer.cpp"
#include "./utils/utility.h"


// execute with
//for((i=1;i<33;i*=2)); do ./parallel2 20000 10 $i; done


using namespace std;


int main(int argc, char * argv[]) {
    if(argc != 5){
        cout << "Usage: matrix_size number_of_iterations number_of_threads check_flag" << endl;
        return (0);
    }

    int n = atoi(argv[1]); // matrix size
    int k = atoi(argv[2]); // number of iterations
    int nw = atoi(argv[3]); // number of threads
    bool check = (atoi(argv[4]) == 0 ? false : true);
    int seed = 123;

    // generate a random linear system
    vector<vector<float>> matrix = GenerateRandomMatrix(n, seed);

    vector<float> acutal_x(n);
    acutal_x = GenerateRandomVector(n, 0.0, 1.0, seed);

    // compute the linear system to find b
    vector<float> b(n);
    b=compute_b(matrix, acutal_x, n);

    // initialize a vector with all zeroes to be used to find a solution using Jacobi
    vector<float> x(n, 0.0), new_x(n, 0.0);

    // callback barrier
    int iterations = k;
    auto on_completion = [&]() noexcept {
        iterations--;
        x=new_x;
    };

    barrier sync_point(nw, on_completion);

    // fucntion executed by the threads
    auto body = [&](int tid){
        while(iterations>0){

            for (int i=tid; i<n; i+=nw){
                float sum = 0;
                for (int j = 0; j < n; j++) {
                    if(j!=i){
                        sum += matrix[i][j] * x[j];
                    }
                }
                new_x[i] = (b[i] - sum) / matrix[i][i];
            }
            sync_point.arrive_and_wait();

        }
    };

    vector<thread> tids(nw);

    long u;
    {
        utimer tpar("Parallel Cyclic", &u);
        for(int tid=0; tid<nw; tid++){
            tids[tid] = thread(body, tid);
        }

        for(int i=0; i<nw; i++)
            tids[i].join();
    }

    ofstream myfile;
    myfile.open ("./results/parallel_row_cyclic_barrier.txt", std::ios_base::app);

    myfile << "Matrix size: " << n <<" iterations: "<< k << "\n";
    myfile << "Workers: " << nw << "\n";
    myfile << "Total execution time: " << u <<" usec\n";
    myfile << "Time per iteration: " << u/k <<" usec\n\n";

    myfile.close();

    cout << "Time per iteration = " << u/k <<" usec"<< endl;

    if (check){
        cout << "MATRIX A" << endl;
        print_matrix(matrix, n);

        cout << "vector b" << endl;
        print_vector(b);

        cout << "vector x" << endl;
        print_vector(x);
    }

    return 0;
}
