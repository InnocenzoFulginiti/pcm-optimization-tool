//
// Created by Jakob on 01/02/2023.
//

#ifndef QCPROP_COMPLEX_HPP
#define QCPROP_COMPLEX_HPP


#include <complex>

class Complex {
public:
    Complex() : value(0, 0) {}
    explicit Complex(std::complex<double> value) : value(value) {}
    Complex(double re, double im) : value(re, im) {}
    Complex(double re) : value(re) {}

    //Write real/imag getters
    double real() {
        return this->value.real();
    }

    double imag() {
        return this->value.imag();
    }

    //Pass through operators from std::complex
    Complex& operator+=(const Complex& other) {
        value += other.value;
        return *this;
    }

    Complex& operator-=(const Complex& other) {
        value -= other.value;
        return *this;
    }

    Complex& operator*=(const Complex& other) {
        value *= other.value;
        return *this;
    }

    Complex& operator/=(const Complex& other) {
        value /= other.value;
        return *this;
    }

    Complex operator+ (const Complex& other) const {
        return Complex(value + other.value);
    }

    Complex operator- (const Complex& other) const {
        return Complex(value - other.value);
    }

    Complex operator* (const Complex& other) const {
        return Complex(value * other.value);
    }

    Complex operator/ (const Complex& other) const {
        return Complex(value / other.value);
    }

    friend std::ostream& operator<<(std::ostream& os, const Complex& complex) {
        os << complex.value.real() << "+" << complex.value.imag() << "i";
        return os;
    }

    bool operator==(const Complex& other) const {
        return Complex(value - other.value).isZero();
    }

    [[nodiscard]] bool isZero() const {
        return std::abs(value) < 1e-10;
    }

    //to_string
    [[nodiscard]] std::string to_string() const {
        return std::to_string(value.real()) + "+" + std::to_string(value.imag()) + "i";
    }
private:
    std::complex<double> value;

};


#endif //QCPROP_COMPLEX_HPP
