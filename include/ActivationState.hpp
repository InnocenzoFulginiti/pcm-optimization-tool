#ifndef QCPROP_ACTIVATIONSTATE_HPP
#define QCPROP_ACTIVATIONSTATE_HPP

enum ActivationState {
    ALWAYS, NEVER, SOMETIMES, UNKNOWN
};

inline std::string to_string(ActivationState a) {
    switch (a) {
        case ALWAYS:
            return "ALWAYS";
        case NEVER:
            return "NEVER";
        case SOMETIMES:
            return "SOMETIMES";
        case UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

#endif //QCPROP_ACTIVATIONSTATE_HPP
