#ifndef POLY_H
#define POLY_H

#include <cstdint>
#include <iostream>

//TODO make a free constructor for Poly
//TODO things that return a Poly should instead take a Poly& as argument;

class Poly {
    public:
        typedef uint64_t Block;
        typedef int Bit;
        static constexpr unsigned BLOCK_SIZE = 64;

        Poly();
        Poly(unsigned numBlocks);

        static Poly fromInt(Block value);

        template<typename Generator>
        static Poly random(unsigned len, Generator& g);
        static Poly random(unsigned len);

        //takes [start, end)
        static Poly fromBlocks(Poly origin, unsigned start, unsigned end);

        Bit bit(unsigned i) const;
        Block block(unsigned i) const;
        int degree() const;
        unsigned size() const;
        unsigned numBlocks() const;
        unsigned numUsedBlocks() const;

        Poly operator+(const Poly& other) const;
        Poly operator-(const Poly& other) const;
        Poly operator*(const Poly& other) const;
        Poly operator&(const Poly& other) const;
        Poly operator|(const Poly& other) const;
        Poly operator^(const Poly& other) const;
        Poly operator<<(int i);
        Poly operator>>(int i);

        int computeDegree();

        Poly leftBlockShifted(unsigned i) const;
        Poly rightBlockShifted(unsigned i) const;

        Poly multiplyNaively(const Poly& other) const;
        Poly multiplyKaratsuba32(const Poly& other) const;
        Poly multiplyKaratsuba16(const Poly& other) const;
        Poly multiplyKaratsuba8(const Poly& other) const;

        void setBit(unsigned i, Bit value);
    private:
        void setBlock(unsigned i, Block value);
        void xorBit(unsigned i, Bit value);

        Poly doMultiplyKaratsuba(const Poly& other, unsigned splitLimit) const;
        Poly doMultiplyKaratsuba8(const Poly& other, unsigned splitLimit) const;
        Poly doMultiplyKaratsuba16(const Poly& other, unsigned splitLimit) const;
        Poly doMultiplyKaratsuba32(const Poly& other, unsigned splitLimit) const;
        Poly doMultiplyKaratsuba64(const Poly& other, unsigned splitLimit) const;
        Poly doMultiplyKaratsubaBig(const Poly& other, unsigned splitLimit) const;

        static constexpr unsigned INLINE_SIZE = 256;
        static constexpr unsigned NUM_INLINE_BLOCKS = INLINE_SIZE / BLOCK_SIZE;

        Block inlineBlocks[NUM_INLINE_BLOCKS] = {0};
        int deg = 0;
};

std::ostream& operator<<(std::ostream& os, const Poly& p);

// Templates definitions

template<typename Generator>
Poly Poly::random(unsigned len, Generator& g) {
    Poly res((len + (BLOCK_SIZE - 1)) / BLOCK_SIZE);

    std::uniform_int_distribution<uint64_t> distrib;

    for(unsigned i = 0; i < len / BLOCK_SIZE; i++) {
        res.setBlock(i, distrib(g));
    }

    Block b = distrib(g);
    b >>= (BLOCK_SIZE - len - 1);

    res.setBlock(len / BLOCK_SIZE, b /*>> (BLOCK_SIZE - len - 1)*/);

    res.setBit(len, 1);
    res.computeDegree();

    return res;
}

#endif //POLY_H
