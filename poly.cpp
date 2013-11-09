#include <random>

#include "poly.h"
#include "bit_utils.h"
#include "utils.h"

/*****************************************************************************\
|*                                Constructors                               *|
\*****************************************************************************/ 

Poly::Poly() {
    for (unsigned i = 0; i < NUM_INLINE_BLOCKS; i++) {
        this->inlineBlocks[i] = 0;
    }
    this->deg = -1;
}

Poly::Poly(unsigned numBlocks) {
    //do nothing with numBlocks for now
    (void) numBlocks;

    for (unsigned i = 0; i < NUM_INLINE_BLOCKS; i++) {
        this->inlineBlocks[i] = 0;
    }
    this->deg = -1;
}

Poly Poly::fromInt(Block value) {
    Poly res;
    res.setBlock(0, value);
    res.computeDegree();
    return res;
}

Poly Poly::random(unsigned len) {
    std::default_random_engine generator;
    generator.seed(getNanoseconds());

    return Poly::random(len, generator);
}

//TODO: check bounds
Poly Poly::fromBlocks(Poly origin, unsigned start, unsigned end) {
    Poly res(end - start);

    for(unsigned i = start; i < end; i++) {
        res.setBlock(i - start, origin.block(i));
    }

    return res;
}

/*****************************************************************************\
|*                                  Accessors                                *|
\*****************************************************************************/ 

Poly::Bit Poly::bit(unsigned i) const {
    return ((this->block(i / BLOCK_SIZE)) >> (i % BLOCK_SIZE)) & 1;
}

Poly::Block Poly::block(unsigned i) const {
    return this->inlineBlocks[i];
}

int Poly::degree() const {
    return deg;
}

unsigned Poly::size() const {
    return this->degree() + 1;
}

unsigned Poly::numBlocks() const {
    return NUM_INLINE_BLOCKS;
}

unsigned Poly::numUsedBlocks() const {
    return (this->size() + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
}

/*****************************************************************************\
|*                                 Operators                                 *|
\*****************************************************************************/ 

Poly Poly::operator+(const Poly& other) const {
    unsigned nBlocks = std::max(this->numUsedBlocks(), other.numUsedBlocks());
    Poly p(nBlocks);

    for (unsigned i = 0; i < nBlocks; i++) {
        p.setBlock(i, this->block(i) ^ other.block(i));
    }

    p.computeDegree();

    return p;
}

Poly Poly::operator-(const Poly& other) const {
    return *this + other;
}

Poly Poly::operator*(const Poly& other) const {
    return this->multiplyKaratsuba32(other);
}

Poly Poly::operator&(const Poly& other) const {
    unsigned nBlocks = (std::max(this->size(), other.size()) + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    Poly p(nBlocks);

    for (unsigned i = 0; i < nBlocks; i++) {
        p.setBlock(i, this->block(i) & other.block(i));
    }

    return p;
}

Poly Poly::operator|(const Poly& other) const {
    unsigned nBlocks = (std::max(this->size(), other.size()) + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    Poly p(nBlocks);

    for (unsigned i = 0; i < nBlocks; i++) {
        p.setBlock(i, this->block(i) | other.block(i));
    }

    return p;
}

Poly Poly::operator^(const Poly& other) const {
    unsigned nBlocks = (std::max(this->size(), other.size()) + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
    Poly p(nBlocks);

    for (unsigned i = 0; i < nBlocks; i++) {
        p.setBlock(i, this->block(i) ^ other.block(i));
    }

    return p;
}

Poly Poly::operator<<(int i) {
    int iMod = i % BLOCK_SIZE;

    // Handle this case separetaly because shifting by more (or equal) than the block size is an undefined op
    // We just shift by blocks
    if (iMod == 0) {
        Poly res = this->leftBlockShifted(i / BLOCK_SIZE);
        res.deg = this->degree() + i;
        return res;
    }

    int resNBlocks = (this->size() + i + BLOCK_SIZE - 1) / BLOCK_SIZE;
    Poly res(resNBlocks);

    int thisIndex = 0;
    int resIndex = i / BLOCK_SIZE;

    Block shiftedLow = this->block(thisIndex) << iMod;
    res.setBlock(resIndex, shiftedLow);
    resIndex ++;

    Block nextHigh = this->block(thisIndex) >> (BLOCK_SIZE - iMod);

    // For each block of res we gather
    //  - The high part of the previous block of this
    //  - The low part of the current block of this
    // We don't need the high part of the last block of this because it is always 0
    while(resIndex < resNBlocks) {
        thisIndex ++;
        res.setBlock(resIndex, res.block(resIndex) | nextHigh);

        shiftedLow = this->block(thisIndex) << iMod;
        nextHigh = this->block(thisIndex) >> (BLOCK_SIZE - iMod);

        res.setBlock(resIndex, res.block(resIndex) | shiftedLow);

        resIndex ++;
    }

    res.deg = this->degree() + i;
    return res;
}

Poly Poly::operator>>(int i) {
    // Same ideas as operator<<
    int iMod = i % BLOCK_SIZE;

    if (iMod == 0) {
        Poly res = this->rightBlockShifted(i / BLOCK_SIZE);
        res.deg = this->degree() - i;
        return res;
    }

    int resNBlocks = (this->size() + i + BLOCK_SIZE - 1) / BLOCK_SIZE;
    Poly res(resNBlocks);

    int nBlocks = this->numBlocks();

    int resIndex = 0;
    int thisIndex = i / BLOCK_SIZE;

    Block nextHigh = this->block(thisIndex) >> iMod;

    thisIndex ++;

    while(thisIndex < nBlocks) {
        res.setBlock(resIndex, res.block(resIndex) | nextHigh);

        Block shiftedLow = this->block(thisIndex) << (BLOCK_SIZE - iMod);
        nextHigh = this->block(thisIndex) >> iMod;

        res.setBlock(resIndex, res.block(resIndex) | shiftedLow);

        resIndex ++;
        thisIndex ++;
    }

    res.setBlock(resIndex, res.block(resIndex) | nextHigh);

    res.deg = this->degree() - i;
    return res;
}

/*****************************************************************************\
|*                                   Misc                                    *|
\*****************************************************************************/ 

int Poly::computeDegree() {
    for (unsigned i = this->numBlocks(); i-->0;) {
        Block b = this->block(i);
        if (b != 0) {
            //TODO remove the assumption on the block size
            deg = i * BLOCK_SIZE + log2_u64(b);
            return deg;
        }
    }

    //We have the 0 polynomial.
    deg = -1;
    return deg;
}

Poly Poly::leftBlockShifted(unsigned i) const {
    unsigned nBlocks = this->numUsedBlocks();
    Poly res(nBlocks + i);

    for (unsigned j = 0; j < nBlocks; j++) {
        res.setBlock(i + j, this->block(j));
    }

    return res;
}

Poly Poly::rightBlockShifted(unsigned i) const {
    int nBlocks = this->numUsedBlocks();
    Poly res(nBlocks - i);

    for (int j = i; j < nBlocks; j++) {
        res.setBlock(j - i, this->block(j));
    }

    return res;
}

Poly Poly::multiplyNaively(const Poly& other) const {
    Poly res;
    for (unsigned i = 0; i < this->size(); i++) {
        for (unsigned j = 0; j < other.size(); j++) {
            res.xorBit(i + j, this->bit(i) & other.bit(j));            
        }
    }

    res.computeDegree();

    return res;
}

Poly Poly::multiplyKaratsuba8(const Poly& other) const {
    return this->doMultiplyKaratsuba(other, 8);
}

Poly Poly::multiplyKaratsuba16(const Poly& other) const {
    return this->doMultiplyKaratsuba(other, 16);
}

Poly Poly::multiplyKaratsuba32(const Poly& other) const {
    return this->doMultiplyKaratsuba(other, 32);
}

/*****************************************************************************\
|*                         Private Basic Operations                          *|
\*****************************************************************************/ 

void Poly::setBlock(unsigned i, Block value) {
    this->inlineBlocks[i] = value;
}

void Poly::setBit(unsigned i, Bit value) {
    //Casting to blocks, else the shifting operations are done on 32 bits (size of the bits)
    //and the ~ puts ones in the upper bits of the block
    //Note that it is free, simply changing the shll to shlq mnemonics.
    Block valueBlock = value;

    this->inlineBlocks[i / BLOCK_SIZE] &= ~(((uint64_t)1) << i);
    this->inlineBlocks[i / BLOCK_SIZE] |= (valueBlock << i);
}

void Poly::xorBit(unsigned i, Bit value) {
    Block valueBlock = value;
    this->inlineBlocks[i / BLOCK_SIZE] ^= (valueBlock << i);
}

/*****************************************************************************\
|*                         Karatsuba implementation                          *|
\*****************************************************************************/ 

Poly Poly::doMultiplyKaratsuba(const Poly& other, unsigned splitLimit) const {
    if (this->size() == 0 or other.size() == 0) {
        return Poly::fromInt(0);
    }

    unsigned nBlocks = std::max(this->numUsedBlocks(), other.numUsedBlocks());
    static const bool debug = false;
    //TODO compute the degree before returning

    if (nBlocks > 1) {
        if (debug) {
            std::cout << "Doing karatsuba big" << std::endl;
        }

        return this->doMultiplyKaratsubaBig(other, splitLimit);
    }

    unsigned size = std::max(this->size(), other.size());

   /* if (size <= 8) {
    }
    */

    if (size <= 16) {
        if (debug) {
            std::cout << "Doing karatsuba 16" << std::endl;
        }
        return this->doMultiplyKaratsuba16(other, splitLimit);
    }

    if (size <= 32) {
        if (debug) {
            std::cout << "Doing karatsuba 32" << std::endl;
        }
        return this->doMultiplyKaratsuba32(other, splitLimit);
    }

    if (debug) {
        std::cout << "Doing karatsuba 64" << std::endl;
    }

    Poly res = this->doMultiplyKaratsuba64(other, splitLimit);
    res.computeDegree();
    return res;
}

Poly Poly::doMultiplyKaratsuba16(const Poly& other, unsigned splitLimit) const {
    return Poly::fromInt(convolution_16_32(this->block(0), other.block(0)));
}

Poly Poly::doMultiplyKaratsuba32(const Poly& other, unsigned splitLimit) const {
    return Poly::fromInt(convolution_32_64(this->block(0), other.block(0)));
}

Poly Poly::doMultiplyKaratsuba64(const Poly& other, unsigned splitLimit) const {
    static const bool debug = false;
    Poly a1 = Poly::fromInt(other.block(0) >> 32);
    Poly a0 = Poly::fromInt(other.block(0) & 0xFFFFFFFF);

    Poly b1 = Poly::fromInt(this->block(0) >> 32);
    Poly b0 = Poly::fromInt(this->block(0) & 0xFFFFFFFF);

    Poly c2 = a1.doMultiplyKaratsuba32(b1, splitLimit);
    Poly c0 = a0.doMultiplyKaratsuba32(b0, splitLimit);
    Poly c1 = (a1 + a0).doMultiplyKaratsuba32(b1 + b0, splitLimit) - c2 - c0;

    if (debug) {
        std::cout << " a: " << other << std::endl;
        std::cout << "a1: " << a1 << std::endl;
        std::cout << "a0: " << a0 << std::endl;

        std::cout << std::endl;

        std::cout << " b: " << *this << std::endl;
        std::cout << "b1: " << b1 << std::endl;
        std::cout << "b0: " << b0 << std::endl;

        std::cout << std::endl;

        std::cout << "c2: " << c2 << std::endl;
        std::cout << "c1: " << c1 << std::endl;
        std::cout << "c0: " << c0 << std::endl;

        std::cout << std::endl;
    }

    Poly res;

    Block res1 = c2.block(0) ^ (c1.block(0) >> 32);
    Block res0 = c0.block(0) ^ ((c1.block(0) & 0xFFFFFFFF) << 32);

    res.setBlock(1, res1);
    res.setBlock(0, res0);

    if (debug) {
        if ((this->multiplyNaively(other) + res).size() != 0) {
            std::cout << "Error in Karatsuba64" << std::endl;
        }
    }

    return res;
}

Poly Poly::doMultiplyKaratsubaBig(const Poly& other, unsigned splitLimit) const {
    unsigned nBlocks = std::max(this->numUsedBlocks(), other.numUsedBlocks());
    static const bool debug = false;
    unsigned cut = (nBlocks + 1) / 2;

    Poly a1 = other.rightBlockShifted(cut);
    Poly a0 = Poly::fromBlocks(other, 0, cut);

    Poly b1 = this->rightBlockShifted(cut);
    Poly b0 = Poly::fromBlocks(*this, 0, cut);

    a1.computeDegree();
    a0.computeDegree();
    b1.computeDegree();
    b0.computeDegree();

    Poly c2 = a1.doMultiplyKaratsuba(b1, splitLimit);
    Poly c0 = a0.doMultiplyKaratsuba(b0, splitLimit);

    Poly c1 = (a1 + a0).doMultiplyKaratsuba(b1 + b0, splitLimit) - c2 - c0;

    if (debug) {
        std::cout << " a: " << other << std::endl;
        std::cout << "a1: " << a1 << std::endl;
        std::cout << "a0: " << a0 << std::endl;

        std::cout << std::endl;

        std::cout << " b: " << *this << std::endl;
        std::cout << "b1: " << b1 << std::endl;
        std::cout << "b0: " << b0 << std::endl;

        std::cout << std::endl;

        std::cout << "c2: " << c2 << std::endl;
        std::cout << "c1: " << c1 << std::endl;
        std::cout << "c0: " << c0 << std::endl;

        std::cout << std::endl;
    }

    Poly c2Shifted = c2.leftBlockShifted(cut * 2);
    Poly c1Shifted = c1.leftBlockShifted(cut);
    
    c2Shifted.computeDegree();
    c1Shifted.computeDegree();

    Poly res = c2Shifted + c1Shifted + c0;
    
    if (debug) {
        if ((this->multiplyNaively(other) + res).size() != 0) {
            std::cout << "#################################################""Error in KaratsubaBig" << std::endl;
        }else {
            std::cout << "Ok" << std::endl;
        }
    }

    return res;
}

/*****************************************************************************\
|*                                      IO                                   *|
\*****************************************************************************/ 

std::ostream& operator<<(std::ostream& os, const Poly& p) {
    os << "(";
    for (int i = p.size(); i-->0;) {
        os << p.bit(i);
        if (i != 0 and i % Poly::BLOCK_SIZE == 0) {
            os << ' ';
        }
    }
    os << "] - " << p.degree();

    return os;
}
