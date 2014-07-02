#include <stdio.h>
#include "ResultDatabase.h"
#include "OptionParser.h"
#include "ProgressBar.h"
#include "Utility.h"


// Forward Declarations
template <class T> inline void Add1(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Add2(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Add4(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Add8(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
extern "C" void Add1Float(float *, int, int, float, double *, double *);
extern "C" void Add1Double(double *, int, int, double, double *, double *);
extern "C" void Add2Float(float *, int, int, float, double *, double *);
extern "C" void Add2Double(double *, int, int, double, double *, double *);
extern "C" void Add4Float(float *, int, int, float, double *, double *);
extern "C" void Add4Double(double *, int, int, double, double *, double *);
extern "C" void Add8Float(float *, int, int, float, double *, double *);
extern "C" void Add8Double(double *, int, int, double, double *, double *);

template <class T> inline void Mul1(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Mul2(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Mul4(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
template <class T> inline void Mul8(T *data, int numFloats, int nIters, T v, 
                                    double *kTime, double *tTime);
extern "C" void Mul1Float(float *, int, int, float, double *, double *);
extern "C" void Mul1Double(double *, int, int, double, double *, double *);
extern "C" void Mul2Float(float *, int, int, float, double *, double *);
extern "C" void Mul2Double(double *, int, int, double, double *, double *);
extern "C" void Mul4Float(float *, int, int, float, double *, double *);
extern "C" void Mul4Double(double *, int, int, double, double *, double *);
extern "C" void Mul8Float(float *, int, int, float, double *, double *);
extern "C" void Mul8Double(double *, int, int, double, double *, double *);

template <class T> inline void MAdd1(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MAdd2(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MAdd4(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MAdd8(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
extern "C" void MAdd1Float(float *, int, int, float, float, double *, double *);
extern "C" void MAdd1Double(double *, int, int, double, double, double *, double *);
extern "C" void MAdd2Float(float *, int, int, float, float, double *, double *);
extern "C" void MAdd2Double(double *, int, int, double, double, double *, double *);
extern "C" void MAdd4Float(float *, int, int, float, float, double *, double *);
extern "C" void MAdd4Double(double *, int, int, double, double, double *, double *);
extern "C" void MAdd8Float(float *, int, int, float, float, double *, double *);
extern "C" void MAdd8Double(double *, int, int, double, double, double *, double *);

template <class T> inline void MulMAdd1(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MulMAdd2(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MulMAdd4(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
template <class T> inline void MulMAdd8(T *data, int numFloats, int nIters, T v, T v2,
                                    double *kTime, double *tTime);
extern "C" void MulMAdd1Float(float *, int, int, float, float, double *, double *);
extern "C" void MulMAdd1Double(double *, int, int, double, double, double *, double *);
extern "C" void MulMAdd2Float(float *, int, int, float, float, double *, double *);
extern "C" void MulMAdd2Double(double *, int, int, double, double, double *, double *);
extern "C" void MulMAdd4Float(float *, int, int, float, float, double *, double *);
extern "C" void MulMAdd4Double(double *, int, int, double, double, double *, double *);
extern "C" void MulMAdd8Float(float *, int, int, float, float, double *, double *);
extern "C" void MulMAdd8Double(double *, int, int, double, double, double *, double *);

// execute simple precision and double precision versions of the benchmarks
template <class T> void
RunTest(ResultDatabase &resultDB, int npasses, int verbose, int quiet, 
        float repeatF, ProgressBar &pb, const char* precision);

void
addBenchmarkSpecOptions(OptionParser &op)
{
   ;
}

void
RunBenchmark(ResultDatabase &resultDB, OptionParser &op)
{
    bool verbose = op.getOptionBool("verbose");
    bool quiet = op.getOptionBool("quiet");
    const unsigned int passes = op.getOptionInt("passes");
    bool doDouble = true;
    double kernelTime, transferTime, t;

    // Initialize host data, with the first half the same as the second
    const unsigned int halfBufSize = 1024*1024;
    unsigned int halfNumFloats = halfBufSize / sizeof(float), numFloats = 2*halfNumFloats;
    float *hostMem;
    hostMem = new float[numFloats];
    for (int j=0; j<halfNumFloats; ++j)
    {
        hostMem[j] = hostMem[numFloats-j-1] = (float)(drand48()*10.0);
    }

    // Benchmark the MulMAdd2 kernel to compute a scaling factor.
    MulMAdd2Float(hostMem, numFloats, 10, 3.75, 0.355, &kernelTime, &transferTime); 
    t = kernelTime * 1.e9;
    double repeatF = 1.1e07 / (double)t;
    fprintf (stdout, "Adjust repeat factor = %lg\n", repeatF);
    delete[] hostMem;

    // Initialize progress bar. We have 16 generic kernels 
    // Each kernel is executed 'passes' number of times for each single precision and
    // double precision (if avaialble).
    int totalRuns = 16*passes;
    if (doDouble) 
       totalRuns <<= 1;  // multiply by 2
    ProgressBar pb(totalRuns);
    if (!verbose && !quiet)
       pb.Show(stdout);

    // Run single precision kernels
    RunTest<float> (resultDB, passes, verbose, quiet, repeatF, pb, "-SP");
    
    // Run double precision kernels
    RunTest<double> (resultDB, passes, verbose, quiet, repeatF, pb, "-DP");

    if (!verbose)
        fprintf (stdout, "\n\n");

}


template <class T> void
RunTest(ResultDatabase &resultDB, int npasses, int verbose, int quiet,
        float repeatF, ProgressBar &pb, const char* precision)
{

    T *hostMem;

    // Alloc host memory
    int halfNumFloats = 1024*1024;
    int numFloats = 2*halfNumFloats;
    hostMem = new T[numFloats];
    char sizeStr[128];
    double kernelTime = 0.0;
    double transferTime = 0.0;

    int realRepeats = (int)round(repeatF*20);
    if (realRepeats < 2)
       realRepeats = 2;

    for (int pass=0 ; pass<npasses ; ++pass)
    {
        // Benchmark each generic kernel. Generate new random numbers for each run.
        
        ////////// Add1 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Add1 kernel
        Add1<T>(hostMem, numFloats, realRepeats, 10.0, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        double flopCount = (double)numFloats * 1 * realRepeats * 1800 * 1;
        double t = kernelTime * 1.e9;
        double gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Add1")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Add2 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Add2 kernel
        Add2<T>(hostMem, numFloats, realRepeats, 10.0, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 1 * realRepeats * 900 * 2;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Add2")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Add4 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Add4 kernel
        Add4<T>(hostMem, numFloats, realRepeats, 10.0, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 1 * realRepeats * 600 * 4;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Add4")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Add8 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Add8 kernel
        Add8<T>(hostMem, numFloats, realRepeats, 10.0, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 1 * realRepeats * 300 * 8;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Add8")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Mul1 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Mul1 kernel
        Mul1<T>(hostMem, numFloats, realRepeats, 1.01, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 1800 * 1;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Mul1")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Mul2 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Mul2 kernel
        Mul2<T>(hostMem, numFloats, realRepeats, 1.01, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 900 * 2;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Mul2")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Mul4 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Mul4 kernel
        Mul4<T>(hostMem, numFloats, realRepeats, 1.01, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 600 * 4;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Mul4")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// Mul8 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the Mul8 kernel
        Mul8<T>(hostMem, numFloats, realRepeats, 1.01, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 300 * 8;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("Mul8")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);


        ////////// MAdd1 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MAdd1 kernel
        MAdd1<T>(hostMem, numFloats, realRepeats, 10, 0.9899, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 1800 * 1;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MAdd1")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MAdd2 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MAdd2 kernel
        MAdd2<T>(hostMem, numFloats, realRepeats, 10, 0.9899, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 900 * 2;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MAdd2")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MAdd4 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MAdd4 kernel
        MAdd4<T>(hostMem, numFloats, realRepeats, 10, 0.9899, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 600 * 4;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MAdd4")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MAdd8 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MAdd8 kernel
        MAdd8<T>(hostMem, numFloats, realRepeats, 10, 0.9899, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 2 * realRepeats * 300 * 8;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MAdd8")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MulMAdd1 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MulMAdd1 kernel
        MulMAdd1<T>(hostMem, numFloats, realRepeats, 3.75, 0.355, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 3 * realRepeats * 1800 * 1;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MulMAdd1")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MulMAdd2 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MulMAdd2 kernel
        MulMAdd2<T>(hostMem, numFloats, realRepeats, 3.75, 0.355, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 3 * realRepeats * 900 * 2;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MulMAdd2")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MulMAdd4 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MulMAdd4 kernel
        MulMAdd4<T>(hostMem, numFloats, realRepeats, 3.75, 0.355, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 3 * realRepeats * 600 * 4;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MulMAdd4")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);

        ////////// MulMAdd8 //////////
        // Initialize host data, with the first half the same as the second
        for (int j=0; j<halfNumFloats; ++j)
        {
            hostMem[j] = hostMem[numFloats-j-1] = (T)(drand48()*10.0);
        }

        // Execute the MulMAdd8 kernel
        MulMAdd8<T>(hostMem, numFloats, realRepeats, 3.75, 0.355, &kernelTime, &transferTime); 

        // flopCount = numFloats(pixels) * flopCount/op * numLoopIters * unrollFactor * numStreams
        flopCount = (double)numFloats * 3 * realRepeats * 300 * 8;
        t = kernelTime * 1.e9;
        gflop = flopCount / (double)(t);

        sprintf (sizeStr, "Size:%07d", numFloats);
        resultDB.AddResult(string("MulMAdd8")+precision, sizeStr, "GFLOPS", gflop);

        // Check the result -- At a minimum the first half of memory
        // should match the second half exactly
        for (int j=0 ; j<halfNumFloats ; ++j)
        {
           if (hostMem[j] != hostMem[numFloats-j-1])
           {
               cout << "Error; hostMem[" << j << "]=" << hostMem[j]
                    << " is different from its twin element hostMem["
                    << (numFloats-j-1) << "]=" << hostMem[numFloats-j-1]
                    <<"; stopping check\n";
               break;
           }
        }

        // update progress bar
        pb.addItersDone();
        if (!verbose && !quiet)
           pb.Show(stdout);
    }

    delete[] hostMem;

}

template <> 
inline void Add1<float>(float *data, int numFloats, int nIters,
                        float v, double *kTime, double *tTime)
{
    Add1Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add1<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Add1Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add2<float>(float *data, int numFloats, int nIters, 
                         float v, double *kTime, double *tTime)
{
    Add2Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add2<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Add2Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add4<float>(float *data, int numFloats, int nIters, 
                         float v, double *kTime, double *tTime)
{
    Add4Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add4<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Add4Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add8<float>(float *data, int numFloats, int nIters, 
                         float v, double *kTime, double *tTime)
{
    Add8Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Add8<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Add8Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul1<float>(float *data, int numFloats, int nIters, 
                        float v, double *kTime, double *tTime)
{
    Mul1Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul1<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Mul1Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul2<float>(float *data, int numFloats, int nIters, 
                        float v, double *kTime, double *tTime)
{
    Mul2Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul2<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Mul2Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul4<float>(float *data, int numFloats, int nIters, 
                        float v, double *kTime, double *tTime)
{
    Mul4Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul4<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Mul4Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul8<float>(float *data, int numFloats, int nIters, 
                        float v, double *kTime, double *tTime)
{
    Mul8Float(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void Mul8<double>(double *data, int numFloats, int nIters, 
                         double v, double *kTime, double *tTime)
{
    Mul8Double(data, numFloats, nIters, v, kTime, tTime);
}

template <> 
inline void MAdd1<float>(float *data, int numFloats, int nIters, 
                         float v, float v2, double *kTime, double *tTime)
{
    MAdd1Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd1<double>(double *data, int numFloats, int nIters, 
                          double v, double v2, double *kTime, double *tTime)
{
    MAdd1Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd2<float>(float *data, int numFloats, int nIters, 
                         float v, float v2, double *kTime, double *tTime)
{
    MAdd2Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd2<double>(double *data, int numFloats, int nIters, 
                          double v, double v2, double *kTime, double *tTime)
{
    MAdd2Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd4<float>(float *data, int numFloats, int nIters, 
                         float v, float v2, double *kTime, double *tTime)
{
    MAdd4Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd4<double>(double *data, int numFloats, int nIters, 
                          double v, double v2, double *kTime, double *tTime)
{
    MAdd4Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd8<float>(float *data, int numFloats, int nIters, 
                         float v, float v2, double *kTime, double *tTime)
{
    MAdd8Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MAdd8<double>(double *data, int numFloats, int nIters, 
                          double v, double v2, double *kTime, double *tTime)
{
    MAdd8Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd1<float>(float *data, int numFloats, int nIters, 
                            float v, float v2, double *kTime, double *tTime)
{
    MulMAdd1Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd1<double>(double *data, int numFloats, int nIters, 
                             double v, double v2, double *kTime, double *tTime)
{
    MulMAdd1Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd2<float>(float *data, int numFloats, int nIters, 
                            float v, float v2, double *kTime, double *tTime)
{
    MulMAdd2Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd2<double>(double *data, int numFloats, int nIters, 
                             double v, double v2, double *kTime, double *tTime)
{
    MulMAdd2Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd4<float>(float *data, int numFloats, int nIters, 
                            float v, float v2, double *kTime, double *tTime)
{
    MulMAdd4Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd4<double>(double *data, int numFloats, int nIters, 
                             double v, double v2, double *kTime, double *tTime)
{
    MulMAdd4Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd8<float>(float *data, int numFloats, int nIters, 
                            float v, float v2, double *kTime, double *tTime)
{
    MulMAdd8Float(data, numFloats, nIters, v, v2, kTime, tTime);
}

template <> 
inline void MulMAdd8<double>(double *data, int numFloats, int nIters, 
                             double v, double v2, double *kTime, double *tTime)
{
    MulMAdd8Double(data, numFloats, nIters, v, v2, kTime, tTime);
}

