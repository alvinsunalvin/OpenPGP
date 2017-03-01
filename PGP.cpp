#include "PGP.h"

unsigned int PGP::partialBodyLen(uint8_t first_octet) const{
    return 1 << (first_octet & 0x1f);
}

uint8_t PGP::read_packet_header(const std::string & data, std::string::size_type & pos, std::string::size_type & length, uint8_t & tag, bool & format, uint8_t & partial) const{
    uint8_t ctb = data[pos];                                        // Name "ctb" came from Version 2 [RFC 1991]
    format = ctb & 0x40;                                            // get packet length type (OLD = false; NEW = true)
    length = 0;
    tag = 0;                                                        // default value (error)

    if (!partial){                                                  // if partial continue packets have not been found
        if (!(ctb & 0x80)){
           throw std::runtime_error("Error: First bit of packet header MUST be 1.");
        }

        if (!format){                                               // Old length type RFC4880 sec 4.2.1
            tag = (ctb >> 2) & 15;                                  // get tag value
            if ((ctb & 3) == 0){                                    // 0 - The packet has a one-octet length. The header is 2 octets long.
                length = static_cast <uint8_t> (data[pos + 1]);
                pos += 2;
            }
            else if ((ctb & 3) == 1){                               // 1 - The packet has a two-octet length. The header is 3 octets long.
                length = toint(data.substr(pos + 1, 2), 256);
                pos += 3;
            }
            else if ((ctb & 3) == 2){                               // 2 - The packet has a four-octet length. The header is 5 octets long.
                length = toint(data.substr(pos + 2, 4), 256);
                pos += 5;
            }
            else if ((ctb & 3) == 3){                               // The packet is of indeterminate length. The header is 1 octet long, and the implementation must determine how long the packet is.
                partial = 1;                                        // set to partial start
                length = data.size() - pos - 1;                     // header is one octet long
                pos += 1;
            }
        }
        else{                                                       // New length type RFC4880 sec 4.2.2
            tag = ctb & 63;                                         // get tag value
            const uint8_t first_octet = static_cast <unsigned char> (data[pos + 1]);
            if (first_octet < 192){                                 // 0 - 191; A one-octet Body Length header encodes packet lengths of up to 191 octets.
                length = first_octet;
                pos += 2;
            }
            else if ((192 <= first_octet) & (first_octet < 223)){   // 192 - 8383; A two-octet Body Length header encodes packet lengths of 192 to 8383 octets.
                length = toint(data.substr(pos + 1, 2), 256) - (192 << 8) + 192;
                pos += 3;
            }
            else if (first_octet == 255){                           // 8384 - 4294967295; A five-octet Body Length header encodes packet lengths of up to 4,294,967,295 (0xFFFFFFFF) octets in length.
                length = toint(data.substr(pos + 2, 4), 256);
                pos += 5;
            }
            else if (224 <= first_octet){                           // unknown; When the length of the packet body is not known in advance by the issuer, Partial Body Length headers encode a packet of indeterminate length, effectively making it a stream.
                partial = 1;                                        // set to partial start
                length = partialBodyLen(first_octet);
                pos += 1;
            }
        }
    }
    else{ // partial continue
        partial = 2;                                                // set to partial continue
        tag = 254;                                                  // set to partial body tag

        if (!format){                                               // Old length type RFC4880 sec 4.2.1
            length = data.size() - pos - 1;                         // header is one octet long
        }
        else{                                                       // New length type RFC4880 sec 4.2.2
            length = partialBodyLen(data[pos + 1]);
        }

        pos += 1;                                                   // header is one octet long
    }

    return tag;
}

Packet::Ptr PGP::read_packet_raw(const bool format, const uint8_t tag, uint8_t & partial, const std::string & data, std::string::size_type & pos, const std::string::size_type & length) const{
    Packet::Ptr out;
    if (partial > 1){
        out = std::make_shared <Partial> ();
    }
    else{
        switch (tag){
            case 0:
                throw std::runtime_error("Error: Tag number MUST NOT be 0.");
                break;
            case 1:
                out = std::make_shared <Tag1> ();
                break;
            case 2:
                out = std::make_shared <Tag2> ();
                break;
            case 3:
                out = std::make_shared <Tag3> ();
                break;
            case 4:
                out = std::make_shared <Tag4> ();
                break;
            case 5:
                out = std::make_shared <Tag5> ();
                break;
            case 6:
                out = std::make_shared <Tag6> ();
                break;
            case 7:
                out = std::make_shared <Tag7> ();
                break;
            case 8:
                out = std::make_shared <Tag8> ();
                break;
            case 9:
                out = std::make_shared <Tag9> ();
                break;
            case 10:
                out = std::make_shared <Tag10> ();
                break;
            case 11:
                out = std::make_shared <Tag11> ();
                break;
            case 12:
                out = std::make_shared <Tag12> ();
                break;
            case 13:
                out = std::make_shared <Tag13> ();
                break;
            case 14:
                out = std::make_shared <Tag14> ();
                break;
            case 17:
                out = std::make_shared <Tag17> ();
                break;
            case 18:
                out = std::make_shared <Tag18> ();
                break;
            case 19:
                out = std::make_shared <Tag19> ();
                break;
            case 60:
                out = std::make_shared <Tag60> ();
                break;
            case 61:
                out = std::make_shared <Tag61> ();
                break;
            case 62:
                out = std::make_shared <Tag62> ();
                break;
            case 63:
                out = std::make_shared <Tag63> ();
                break;
            default:
                throw std::runtime_error("Error: Tag not defined.");
                break;
        }
    }

    // fill in data
    out -> set_tag(tag);
    out -> set_format(format);
    out -> set_partial(partial);
    out -> set_size(length);
    out -> read(data.substr(pos, length));

    // update position to end of packet
    pos += length;

    if (partial){
        partial = 2;
    }

    return out;
}

Packet::Ptr PGP::read_packet(const std::string & data, std::string::size_type & pos, uint8_t & partial) const{
    if (pos >= data.size()){
        return nullptr;
    }

    // set in read_packet_header, used in read_packet_raw
    bool format;
    uint8_t tag = 0;
    std::string::size_type length;

    read_packet_header(data, pos, length, tag, format, partial);    // pos is moved past header
    return read_packet_raw(format, tag, partial, data, pos, length);
}

std::string PGP::format_string(std::string data, uint8_t line_length) const{
    std::string out = "";
    for(unsigned int i = 0; i < data.size(); i += line_length){
        out += data.substr(i, line_length) + "\n";
    }
    return out;
}

PGP::PGP()
    : armored(true),
      ASCII_Armor(static_cast <uint8_t> (-1)),
      Armor_Header(),
      packets()
{}

PGP::PGP(const PGP & copy)
    : armored(copy.armored),
      ASCII_Armor(copy.ASCII_Armor),
      Armor_Header(copy.Armor_Header),
      packets(copy.get_packets_clone())
{}

PGP::PGP(const std::string & data)
    : PGP()
{
    read(data);
}

PGP::PGP(std::istream & stream)
    : PGP()
{
    read(stream);
}

PGP::~PGP(){
    packets.clear();
}

void PGP::read(const std::string & data){
    std::stringstream s(data);
    read(s);
}

void PGP::read(std::istream & stream){
    // find armor header
    //
    // 6.2. Forming ASCII Armor
    //     ...
    //     Note that all these Armor Header Lines are to consist of a complete
    //     line. That is to say, there is always a line ending preceding the
    //     starting five dashes, and following the ending five dashes. The
    //     header lines, therefore, MUST start at the beginning of a line, and
    //     MUST NOT have text other than whitespace following them on the same
    //     line. These line endings are considered a part of the Armor Header
    //     Line for the purposes of determining the content they delimit.
    //
    std::string line;
    while (std::getline(stream, line) && line.substr(0, 15) != "-----BEGIN PGP ");

    // if no armor header found, assume entire stream is key
    if (!stream){
        stream.clear();
        stream.seekg(stream.beg);

        // parse entire stream
        read_raw(stream);

        armored = false;
    }
    else{
        // parse armor header
        uint8_t new_ASCII_Armor;
        for(new_ASCII_Armor = 0; new_ASCII_Armor < 7; new_ASCII_Armor++){
            if (("-----BEGIN PGP " + ASCII_Armor_Header[new_ASCII_Armor] + "-----") == line){
                break;
            }
        }

        // Cleartext Signature Framework
        if (new_ASCII_Armor == 6){
            throw std::runtime_error("Error: Data contains message section. Use PGPCleartextSignature to parse this data.");
        }

        // if ASCII Armor was set before calling read()
        if (ASCII_Armor != static_cast <uint8_t> (-1)){
            if (ASCII_Armor != new_ASCII_Armor){
                std::cerr << "Warning: ASCII Armor does not match data type: " << (int) new_ASCII_Armor << std::endl;
            }
        }

        // read Armor Key(s)
        while (std::getline(stream, line) && line.size()){
            std::stringstream s(line);
            std::string key, value;

            if (!(std::getline(s, key, ':') && std::getline(s, value))){
                std::cerr << "Warning: Discarding bad Armor Header: " << line << std::endl;
                continue;
            }

            bool found = false;
            for(std::string const & header_key : ASCII_Armor_Key){
                if (header_key == key){
                    found = true;
                    break;
                }
            }

            if (!found){
                std::cerr << "Warning: Unknown ASCII Armor Header Key \"" << key << "\"." << std::endl;
            }

            Armor_Header.push_back(std::make_pair(key, value));
        }

        // read up to tail
        std::string body;
        while (std::getline(stream, line) && (line.substr(0, 13) != "-----END PGP ")){
            body += line;
        }

        // check for a checksum
        if (body[body.size() - 5] == '='){
            uint32_t checksum = toint(radix642ascii(body.substr(body.size() - 4, 4)), 256);
            body = radix642ascii(body.substr(0, body.size() - 5));
            // check if the checksum is correct
            if (crc24(body) != checksum){
                std::cerr << "Warning: Given checksum does not match calculated value." << std::endl;
            }
        }
        else{
            body = radix642ascii(body);
            std::cerr << "Warning: No checksum found." << std::endl;
        }

        // parse data
        read_raw(body);

        armored = true;
    }
}

void PGP::read_raw(const std::string & data){
    // read each packet
    uint8_t partial = 0;
    std::string::size_type pos = 0;
    while (pos < data.size()){
        Packet::Ptr packet = read_packet(data, pos, partial);
        if (packet){
            packets.push_back(packet);
        }
    }

    if (packets.size()){
        if (partial){                         // last packet must have been a partial packet
            packets.back() -> set_partial(3); // set last partial packet to partial end
        }
    }

    armored = false;                          // assume data was not armored, since it was submitted through this function
}

void PGP::read_raw(std::istream & stream){
    read_raw(std::string(std::istreambuf_iterator <char> (stream), {}));
}

std::string PGP::show(const uint8_t indents, const uint8_t indent_size) const{
    std::stringstream out;
    for(Packet::Ptr const & p : packets){
        out << p -> show(indents, indent_size) << "\n";
    }
    return out.str();
}

std::string PGP::raw(const uint8_t header) const{
    std::string out = "";
    for(Packet::Ptr const & p : packets){
        out += p -> write(header);
    }
    return out;
}

std::string PGP::write(const uint8_t armor, const uint8_t header) const{
    std::string packet_string = raw(header);   // raw PGP data = binary, no ASCII headers
    if ((armor == 1) || (!armor && !armored)){ // if no armor or if default, and not armored
        return packet_string;                  // return raw data
    }
    std::string out = "-----BEGIN PGP " + ASCII_Armor_Header[ASCII_Armor] + "-----\n";
    for(std::pair <std::string, std::string> const & key : Armor_Header){
        out += key.first + ": " + key.second + "\n";
    }
    out += "\n";
    return out + format_string(ascii2radix64(packet_string), MAX_LINE_LENGTH) + "=" + ascii2radix64(unhexlify(makehex(crc24(packet_string), 6))) +  "\n-----END PGP " + ASCII_Armor_Header[ASCII_Armor] + "-----\n";
}

bool PGP::get_armored() const{
    return armored;
}

uint8_t PGP::get_ASCII_Armor() const{
    return ASCII_Armor;
}

PGP::Armor_Header_T PGP::get_Armor_Header() const{
    return Armor_Header;
}

PGP::Packets_T PGP::get_packets() const{
    return packets;
}

PGP::Packets_T PGP::get_packets_clone() const{
    std::vector <Packet::Ptr> out;
    for(Packet::Ptr const & p : packets){
        out.push_back(p -> clone());
    }
    return out;
}

void PGP::set_armored(const bool a){
    armored = a;
}

void PGP::set_ASCII_Armor(const uint8_t armor){
    ASCII_Armor = armor;
    armored = true;
}

void PGP::set_Armor_Header(const PGP::Armor_Header_T & header){
    Armor_Header = header;
}

void PGP::set_packets(const PGP::Packets_T & p){
    packets.clear();
    for(Packet::Ptr const & t : p){
        packets.push_back(t -> clone());
    }
}

PGP & PGP::operator=(const PGP & copy){
    armored = copy.armored;
    ASCII_Armor = copy.ASCII_Armor;
    Armor_Header = copy.Armor_Header;
    packets = copy.get_packets_clone();
    return *this;
}