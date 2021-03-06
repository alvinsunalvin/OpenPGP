#include "Packets/Tag2/Sub19.h"

namespace OpenPGP {
namespace Subpacket {
namespace Tag2 {

Sub19::Sub19(...) {
    throw std::runtime_error("Error: Reserved Subpacket.");
}

Sub::Ptr Sub19::clone() const {
    return std::make_shared <Sub19> (*this);
}

}
}
}
