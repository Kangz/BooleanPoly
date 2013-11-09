#include <chrono>
#include <random>
#include <vector>
#include "poly.h"
#include "utils.h"


void bench_multiply() {
    std::default_random_engine generator;
    generator.seed(getNanoseconds());

    std::uniform_int_distribution<int> degreeDistrib(0, 255);

    std::vector<Poly> polys;

    for (int i = 0;  i < 1000; i++) {
        polys.push_back(Poly::random(degreeDistrib(generator), generator));
    }

    // 1 - Check Correctness of karatsubas
    {
        int tries = 0;
        int successes = 0;

        for (Poly p : polys) {
            for (Poly q : polys) {
                if (p.degree() + q.degree() >= 256) {
                    continue;
                }
                tries ++;

                Poly res1 = p.multiplyNaively(q);
                Poly res2 = p.multiplyKaratsuba32(q);

                if ((res1 + res2).size() == 0) {
                    successes ++;
                }
            }
        }

        std::cout << "Karatsuba32 success ratio : (" << successes << "/" << tries << ")" << std::endl;
    }

    // 2 - Bench the naive method
    {
        int forceBench = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (Poly p : polys) {
            for (Poly q : polys) {
                if (p.degree() + q.degree() >= 256) {
                    continue;
                }
                Poly r = p.multiplyNaively(q);
                forceBench += r.degree();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

        volatile int forceBench2 = forceBench;
        (void) forceBench2;

        std::cout << "Naive took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    // 3 - Bench the karatsuba32 method
    {
        int forceBench = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (Poly p : polys) {
            for (Poly q : polys) {
                if (p.degree() + q.degree() >= 256) {
                    continue;
                }
                Poly r = p.multiplyKaratsuba32(q);
                forceBench += r.degree();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

        volatile int forceBench2 = forceBench;
        (void) forceBench2;

        std::cout << "Karatsuba32 took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }
}

Poly naiveShiftLeft(const Poly& p, int i) {
    Poly res(p.size() + i);
    for (unsigned j = 0; j < p.size(); j++) {
        res.setBit(i + j, p.bit(j));
    }
    res.computeDegree();
    return res;
}

Poly naiveShiftRight(const Poly& p, int i) {
    Poly res(p.size() - i);
    for (unsigned j = i; j < p.size(); j++) {
        res.setBit(j - i, p.bit(j));
    }
    res.computeDegree();
    return res;
}

void bench_shifts() {
    std::default_random_engine generator;
    generator.seed(getNanoseconds());

    std::uniform_int_distribution<int> degreeDistrib(0, 255);

    std::vector<Poly> polys;

    for (int i = 0;  i < 10000; i++) {
        polys.push_back(Poly::random(degreeDistrib(generator), generator));
    }

    // 1 - Check Correctness of shifts
    {
        int tries = 0;
        int successes = 0;

        for (Poly p : polys) {
            for (unsigned i = 0; i < 256 - p.size(); i++) {
                tries ++;

                Poly res1 = naiveShiftLeft(p, i);
                Poly res2 = p << i;

                if ((res1 + res2).size() == 0) {
                    successes ++;
                }
            }

            for (unsigned i = 0;  i <= p.size(); i++) {
                tries ++;

                Poly res1 = naiveShiftRight(p, i);
                Poly res2 = p >> i;

                if ((res1 + res2).size() == 0) {
                    successes ++;
                }
            }
        }

        std::cout << "Shifts success ratio : (" << successes << "/" << tries << ")" << std::endl;
    }

    // 2 - Bench the naive method
    {
        int forceBench = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (Poly p : polys) {
            for (unsigned i = 0; i < 256 - p.size(); i++) {
                Poly r = naiveShiftLeft(p, i);
                forceBench += r.degree();
            }

            for (unsigned i = 0;  i <= p.size(); i++) {
                Poly r = naiveShiftRight(p, i);
                forceBench += r.degree();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

        volatile int forceBench2 = forceBench;
        (void) forceBench2;

        std::cout << "Naive took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }

    // 3 - Bench the fast method
    {
        int forceBench = 0;

        auto start = std::chrono::high_resolution_clock::now();
        for (Poly p : polys) {
            for (unsigned i = 0; i < 256 - p.size(); i++) {
                Poly r = p << i;
                forceBench += r.degree();
            }

            for (unsigned i = 0;  i <= p.size(); i++) {
                Poly r = p >> i;
                forceBench += r.degree();
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

        volatile int forceBench2 = forceBench;
        (void) forceBench2;

        std::cout << "Naive took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }
}

int main(){
    bench_multiply();
    bench_shifts();
}
