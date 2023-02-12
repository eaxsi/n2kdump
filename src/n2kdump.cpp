#include <iostream>
#include <string>
#include <vector>
#include <sstream>


struct canframe{
    uint32_t id;
    uint8_t length;
    std::vector<uint8_t> msg;
};

struct J1939ID{
    uint16_t sa; // source address
    uint16_t ps;
    uint16_t pf;
    uint8_t pg;
    uint32_t pgn;
    bool is_proprietary;
};

canframe parse_line(std::string line)
{
    canframe frame;
    std::stringstream ss(line);
    std::string word;
    int i = 0;
    while (ss >> word) {
        if(i == 1)
            frame.id = std::stoul(word, nullptr, 16);
        else if(i == 2)
            frame.length = static_cast<uint8_t>(word[1]);
        else if(i >= 3)
            frame.msg.push_back(std::stoul(word, nullptr, 16));
        i++;
    }
    return frame;
}

J1939ID decode_j1939_id(uint32_t id)
{
    J1939ID j1939id;
    j1939id.sa = static_cast<uint8_t>(id & 0xFF); // source address
    j1939id.ps = (id >> 8) & 0xFF; // PDU Specific,--> msg specified to adr
    j1939id.pf = (id >> 16) & 0xFF; // PDU Format
    j1939id.is_proprietary = false;

    // Form pgn
    switch(j1939id.pf){
        case 0xEF: // Poropietari
        case 0xFF: // Poropietari
        {
            j1939id.pgn = (j1939id.pf << 8);
            j1939id.is_proprietary = true;
             break;
        }
        case 0xEE: //ISO Address Claim
        case 0xEA: //ISO Request
        case 0xE8: //ISO Acknowledgment
        {
            j1939id.pgn = (j1939id.pf << 8);
            break;
        }
        default:
        {
            j1939id.pgn = (j1939id.pf << 8) + j1939id.ps;
            break;
        }
    }
    return j1939id;
}

int main()
{
    std::string lineInput;
    while (std::getline(std::cin,lineInput))
    {
        canframe frame;
        frame = parse_line(lineInput);
        if(frame.id > 0x800)
        {
            J1939ID j1939id = decode_j1939_id(frame.id);
            if(j1939id.is_proprietary)
            {
                uint8_t mi_low = frame.msg.at(0);
                uint8_t mi_high = frame.msg.at(1);
                uint16_t mi = mi_low + (mi_high<<8);

                uint16_t man = mi & 0x7FF;
                //uint16_t ind_grp = (mi >> 13) & 0xF;

                // Decode special proprietary message if manufacturer has specified it
                switch(man){
                    case 358:
                    {
                        uint16_t cmd = frame.msg.at(2) + (frame.msg.at(3)<<8);
                        uint16_t data = frame.msg.at(4) + (frame.msg.at(5)<<8);
                        switch(cmd){
                            case 0x0001: // VREQ
                            {
                                std::cout << "Node 0x" << std::hex << j1939id.sa;
                                std::cout << " requesting register 0x" << data;
                                std::cout << " from 0x" << j1939id.ps << std::endl;
                                break;
                            }
                            default:
                            {
                                std::cout << "Victron register action from 0x" << j1939id.sa << std::endl;
                                break;
                            }
                        }                        
                        break;
                    }
                    default:
                    {
                        if(j1939id.ps == 0xFF)
                        {
                            std::cout << "Proprietary broadcast from 0x" << j1939id.sa;
                            std::cout << ", " << "manufacturer: " << std::dec << man << std::endl;
                        }
                        else
                        {
                            std::cout << "Proprietary message from 0x" << j1939id.sa;
                            std::cout << " to 0x" << j1939id.ps << " manufacturer: ";
                            std::cout << std::dec << man << std::endl;
                        }
                        break;
                    }
                }
            }
            else
            {
                switch(j1939id.pgn){
                    case 0xEA00: // ISO Request
                    {
                        std::cout << "ISO Request";
                        std::cout << " from: 0x" << std::hex << j1939id.sa;
                        std::cout << " to: 0x" << j1939id.ps << std::endl;
                        break;
                    }
                    case 0xEE00: // ISO Address Claim
                    {
                        std::cout << "ISO Address Claim";
                        std::cout << " from: 0x" << std::hex << j1939id.sa;
                        std::cout << " to: 0x" << j1939id.ps << std::endl;
                        break;
                    }
                    case 0xE800: // ISO Acknowledgment
                    {
                        std::cout << "ISO Acknowledgment";
                        std::cout << " from: 0x" << std::hex << j1939id.sa;
                        std::cout << " to: 0x" << j1939id.ps << std::endl;
                        break;
                    }
                    default:
                    {
                        std::cout << "PGN: 0x" << j1939id.pgn;
                        std::cout << " from: 0x" << std::hex << j1939id.sa;
                        std::cout << " to: 0x" << j1939id.ps << std::endl;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
