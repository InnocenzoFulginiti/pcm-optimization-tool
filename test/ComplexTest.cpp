//
// Created by zuchna on 2/3/23.
//

#include <catch2/catch_test_macros.hpp>

#include "Complex.hpp"

TEST_CASE("Create a Complex Number") {
    Complex a = 1;
    REQUIRE(a.real() == 1.0);
    REQUIRE(a.imag() == 0.0);

    //Try out all constructors of Complex
    Complex b(2, 3);
    REQUIRE(b.real() == 2.0);
    REQUIRE(b.imag() == 3.0);

    Complex c(4);
    REQUIRE(c.real() == 4.0);
    REQUIRE(c.imag() == 0.0);

    Complex d(std::complex<double>(5, 6));
    REQUIRE(d.real() == 5.0);
    REQUIRE(d.imag() == 6.0);
}

TEST_CASE("Complex Operators") {
    //Test Complex Operators
    Complex a(1, 2);
    Complex b(3, 4);

    Complex c = a + b;
    REQUIRE(c.real() == 4.0);
    REQUIRE(c.imag() == 6.0);

    c = a - b;
    REQUIRE(c.real() == -2.0);
    REQUIRE(c.imag() == -2.0);

    c = a * b;
    REQUIRE(c.real() == -5.0);
    REQUIRE(c.imag() == 10.0);

    c = a / b;
    REQUIRE(c.real() == 0.44);
    REQUIRE(c.imag() == 0.08);

    c = a;
    c += b;
    REQUIRE(c.real() == 4.0);
    REQUIRE(c.imag() == 6.0);

    c = a;
    c -= b;
    REQUIRE(c.real() == -2.0);
    REQUIRE(c.imag() == -2.0);

    c = a;
    c *= b;
    REQUIRE(c.real() == -5.0);
    REQUIRE(c.imag() == 10.0);

    c = a;
    c /= b;
    REQUIRE(c.real() == 0.44);
    REQUIRE(c.imag() == 0.08);
}

TEST_CASE("Getters and Setters") {
    Complex a(1, 2);
    REQUIRE(a.real() == 1.0);
    REQUIRE(a.imag() == 2.0);

    a.real(3);
    a.imag(4);
    REQUIRE(a.real() == 3.0);
    REQUIRE(a.imag() == 4.0);
}

TEST_CASE("Compare Complex Numbers") {
    Complex a(1, 2);
    Complex b(1, 2);
    Complex c(3, 4);

    REQUIRE(a == b);
    REQUIRE(a != c);
}