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
        Poly operator&(const Poly& other) const;
        Poly operator|(const Poly& other) const;
        Poly operator^(const Poly& other) const;

        int computeDegree();

        Poly leftBlockShifted(unsigned i) const;
        Poly rightBlockShifted(unsigned i) const;

        Poly multiplyNaively(const Poly& other) const;
        Poly multiplyKaratsuba32(const Poly& other) const;
        Poly multiplyKaratsuba16(const Poly& other) const;
        Poly multiplyKaratsuba8(const Poly& other) const;

    private:
        void setBlock(unsigned i, Block value);
        void setBit(unsigned i, Bit value);
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


#endif //POLY_H
