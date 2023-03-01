//
// Created by Jakob on 01/02/2023.
//
#pragma once


#include <complex>

class Complex {
public:
    Complex() : value(0, 0) {}
    explicit Complex(std::complex<double> value) : value(value) {}
    Complex(double re, double im) : value(re, im) {}
    Complex(double re) : value(re) {}

    //Write real/imag getters and setters
    void real(double re) {
        this->value.real(re);
    }

    void imag(double im) {
        this->value.imag(im);
    }

    double real() const {
        return this->value.real();
    }

    double imag() const {
        return this->value.imag();
    }

    //Norm
    [[nodiscard]] double norm() const {
        return std::norm(value);
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

    bool operator!=(const Complex& other) const {
        return !(*this == other);
    }

    [[nodiscard]] bool isZero() const {
        return std::abs(value) < 1e-10;
    }

    [[nodiscard]] Complex exp() const {
        return Complex(std::exp(this->value));
    }


    //to_string
    [[nodiscard]] std::string to_string() const {
        return std::to_string(value.real()) + ((value.imag() < 0)? "" : "+") + std::to_string(value.imag()) + "i";
    }
private:
    std::complex<double> value;

};
