//
// Created by Jakob on 01/02/2023.
//
#pragma once


#include <complex>

class Complex {
public:
    Complex() : value(0, 0) {}
    explicit Complex(std::complex<double> clone) : value(clone) {}
    Complex(double re, double im) : value(re, im) {}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    Complex(double re) : value(re) {}
#pragma clang diagnostic pop

    //Write real/imag getters and setters
    void real(double re) {
        this->value.real(re);
    }

    void imag(double im) {
        this->value.imag(im);
    }

    [[nodiscard]] double real() const {
        return this->value.real();
    }

    [[nodiscard]] double imag() const {
        return this->value.imag();
    }

    /**
     * Returns the magnitude
     * @return
     */

    [[nodiscard]] double abs() const {
        return std::abs(value);
    }

    /**
     * Returns the squared magnitude
     * @return
     */
    [[nodiscard]] double norm() const {
        return std::norm(value);
    }

    /**
     * Returns the phase angle
     * @return
     */
    [[nodiscard]] double arg() const {
        return std::arg(value);
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
        return this->abs() <= epsilon;
    }

    [[nodiscard]] Complex exp() const {
        return Complex(std::exp(this->value));
    }

    //to_string
    [[nodiscard]] std::string to_string() const {
        return std::to_string(value.real()) + ((value.imag() < 0) ? "" : "+") + std::to_string(value.imag()) + "i";
    }

    /**
     * Sets epsilon, value used to decide if a complex number is zero or
     * two numbers are equal
     * @param epsilon
     */
    static void setEpsilon(double eps) {
        Complex::epsilon = eps;
    }

    [[nodiscard]] static double getEpsilon() {
        return epsilon;
    }

private:
    static double epsilon;
    std::complex<double> value;

};
