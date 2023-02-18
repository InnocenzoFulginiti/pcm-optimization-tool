//
// Created by zuchna on 2/17/23.
//

#include "../include/CompoundGateInliner.hpp"

qc::QuantumComputation CompoundGateInliner::optimize(qc::QuantumComputation &qc) const {
    auto ret = qc.clone();
    ret.clear();

    for (auto &op : qc) {
        addOperation(ret, op,  {});
    }

    return ret;
}

void CompoundGateInliner::addOperation(qc::QuantumComputation &qc,
                                       const std::unique_ptr<qc::Operation>& op,
                                       const std::vector<qc::Control>& controls) {
    if(op->isCompoundOperation()) {
        auto compoundOp = dynamic_cast<qc::CompoundOperation*>(op.get());
        auto compoundControls = compoundOp->getControls();

        std::vector<qc::Control> subControls;

        for (auto superControl : controls) {
            subControls.emplace_back(superControl);
        }

        for (auto &control : controls) {
            subControls.emplace_back(control);
        }

        for (const auto &subOp : *compoundOp) {
            addOperation(qc, subOp, subControls);
        }
    } else {
        //Add super Controls to Operations
        auto newOp = op->clone();
        for(auto c : controls) {
            newOp->getControls().insert(c);
        }

        qc.emplace_back(std::move(newOp));
    }
}
