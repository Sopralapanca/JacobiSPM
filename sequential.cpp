#include <iostream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <algorithm>

#include "./utils/utimer.cpp"

/*
The Jacobi iterative method computes the result of a system of equations
Ax = B (with x vector of variable of length n, A matrix of coefficients of dimension n by n
and B vector of known terms of length n)
iteratively computing a new approximation of the values of the different variables according to the
formula:

x_i^(k+1) = (1/a_ii) * (b_i - sum_j>i a_ij*x_j^k) for all i = 1, ..., n

starting from some initial assignment of each xi (e.g. 0).
We require to implement the Jacobi method with both native C++ threads and FastFlow.

 */

// per vedere se è vettorizzabile
// g++ sequential.cpp -O3 -funroll-loops -ftree-vectorize -fopt-info-vec-all -o sequential |& grep sequential
// è una map secca, dovrebbe avere uno speedup lineare dove c'è l'overhead?

using namespace std;

random_device rd;
mt19937 gen(rd());

void print_matrix(vector<vector<double>> matrix, int size){
    // Printing the  matrix
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++)
            cout << matrix[i][j] << " ";
        cout << endl;
    }
}

void print_vector(vector<double> vector){
    // Printing the  matrix
    for (int i = 0; i < vector.size(); i++){
        cout << vector[i] << " ";
        cout << endl;
    }
}


vector<double> GenerateRandomVector(int size,int min, int max, int seed) {
    gen.seed(seed);

    vector<double> vec(size);
    uniform_real_distribution<> dis(min, max);
    generate(vec.begin(), vec.end(), [&](){ return dis(gen); });
    return vec;
}

vector<double> compute_b(vector<vector<double>> a, vector<double> x, int size) {
    vector<double> b(size);
    int i, j, sum;
    for (i=0;i<size;i++){
        sum=0;
        for(j=0;j<size;j++){
            sum+=a[i][j]*x[j];
        }
        b[i]=sum;
    }

    return b;
}

vector<vector<double>> GenerateRandomMatrix(int n, int seed){
    // given the initial seed, generate a vector of random numbers
    // representing seeds to randomly generate vectors for the matrix
    vector<int> seeds_vector(n);
    gen.seed(seed);
    uniform_int_distribution<int> dis(0, 10000);
    generate(seeds_vector.begin(), seeds_vector.end(), [&](){ return dis(gen); });

    vector<vector<double>> matrix(n, vector<double>(n));
    for( int i = 0; i < n; i++){
        matrix[i] = GenerateRandomVector(n, 0.0, 10.0, seeds_vector[i]);
    }
    return matrix;
}

int main(int argc, char * argv[]) {
    if(argc != 3){
        cout << "Usage: matrix_size number_of_iterations" << endl;
        return (0);
    }

    int n = atoi(argv[1]); // matrix size
    int k = atoi(argv[2]); // number of iterations
    int seed = 123; //int seed = atoi(argv[3]);


    // try to vectorize the sequential part, make a comparison between vectorized and not vectorizer
    // controllare se vettorizza

    // generate a random linear system
    vector<vector<double>> matrix = GenerateRandomMatrix(n, seed);

    vector<double> real_x(n);
    real_x = GenerateRandomVector(n, 0.0, 1.0, seed);

    // compute the linear system to find b
    vector<double> b(n);
    b=compute_b(matrix, real_x, n);

    // initialize a vector with all zeroes to be used to find a solution using Jacobi
    vector<double> x(n, 0.0);

    long u;
    {
        utimer tseq("Seq", &u);
        // numero di iterazioni
        for (int it=0; it<k; it++) {

            //iterate over b and x
            for(int i=0; i<n;i++) {
                //compute sum
                int sum = 0;
                for (int j = i + 1; j < n; j++) {
                    sum = matrix[i][j] * x[j];
                }

                x[i] = (b[i] - sum) / matrix[i][i];
            }
        }
    }
    // fix
    /*
    ofstream myfile;
    myfile.open ("sequential_times.txt");

    myfile << "Total execution time: " << u <<"usec\n";
    myfile << "Time per iteration: " << u/k <<"usec\n";

    myfile.close();
    */


    cout << "Total execution time: " << u <<"usec" << endl;
    cout << "Time per iteration = " << u/k <<"usec"<< endl;


    // compute some overheads

    return 0;
}
