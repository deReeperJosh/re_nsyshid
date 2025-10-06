#include "Dimensions.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <span>

#include "utils/FSUtils.hpp"
#include "utils/logger.h"

#include "utils/aes.hpp"
#include "utils/sha1.h"

static constexpr std::array<uint8_t, 16> COMMAND_KEY = {0x55, 0xFE, 0xF6, 0xB0, 0x62, 0xBF, 0x0B, 0x41,
                                                        0xC9, 0xB3, 0x7C, 0xB4, 0x97, 0x3E, 0x29, 0x7B};

static constexpr std::array<uint8_t, 17> CHAR_CONSTANT = {0xB7, 0xD5, 0xD7, 0xE6, 0xE7, 0xBA, 0x3C, 0xA8,
                                                          0xD8, 0x75, 0x47, 0x68, 0xCF, 0x23, 0xE9, 0xFE, 0xAA};

static constexpr std::array<uint8_t, 25> PWD_CONSTANT = {0x28, 0x63, 0x29, 0x20, 0x43, 0x6F, 0x70, 0x79,
                                                         0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x4C, 0x45,
                                                         0x47, 0x4F, 0x20, 0x32, 0x30, 0x31, 0x34, 0xAA, 0xAA};

DimensionsToypad g_dimensionstoypad;

const std::map<const uint32_t, const char *> s_listMinis = {
        {0, "Blank Tag"},
        {1, "Batman"},
        {2, "Gandalf"},
        {3, "Wyldstyle"},
        {4, "Aquaman"},
        {5, "Bad Cop"},
        {6, "Bane"},
        {7, "Bart Simpson"},
        {8, "Benny"},
        {9, "Chell"},
        {10, "Cole"},
        {11, "Cragger"},
        {12, "Cyborg"},
        {13, "Cyberman"},
        {14, "Doc Brown"},
        {15, "The Doctor"},
        {16, "Emmet"},
        {17, "Eris"},
        {18, "Gimli"},
        {19, "Gollum"},
        {20, "Harley Quinn"},
        {21, "Homer Simpson"},
        {22, "Jay"},
        {23, "Joker"},
        {24, "Kai"},
        {25, "ACU Trooper"},
        {26, "Gamer Kid"},
        {27, "Krusty the Clown"},
        {28, "Laval"},
        {29, "Legolas"},
        {30, "Lloyd"},
        {31, "Marty McFly"},
        {32, "Nya"},
        {33, "Owen Grady"},
        {34, "Peter Venkman"},
        {35, "Slimer"},
        {36, "Scooby-Doo"},
        {37, "Sensei Wu"},
        {38, "Shaggy"},
        {39, "Stay Puft"},
        {40, "Superman"},
        {41, "Unikitty"},
        {42, "Wicked Witch of the West"},
        {43, "Wonder Woman"},
        {44, "Zane"},
        {45, "Green Arrow"},
        {46, "Supergirl"},
        {47, "Abby Yates"},
        {48, "Finn the Human"},
        {49, "Ethan Hunt"},
        {50, "Lumpy Space Princess"},
        {51, "Jake the Dog"},
        {52, "Harry Potter"},
        {53, "Lord Voldemort"},
        {54, "Michael Knight"},
        {55, "B.A. Baracus"},
        {56, "Newt Scamander"},
        {57, "Sonic the Hedgehog"},
        {58, "Future Update (unreleased)"},
        {59, "Gizmo"},
        {60, "Stripe"},
        {61, "E.T."},
        {62, "Tina Goldstein"},
        {63, "Marceline the Vampire Queen"},
        {64, "Batgirl"},
        {65, "Robin"},
        {66, "Sloth"},
        {67, "Hermione Granger"},
        {68, "Chase McCain"},
        {69, "Excalibur Batman"},
        {70, "Raven"},
        {71, "Beast Boy"},
        {72, "Betelgeuse"},
        {73, "Lord Vortech (unreleased)"},
        {74, "Blossom"},
        {75, "Bubbles"},
        {76, "Buttercup"},
        {77, "Starfire"},
        {78, "World 15 (unreleased)"},
        {79, "World 16 (unreleased)"},
        {80, "World 17 (unreleased)"},
        {81, "World 18 (unreleased)"},
        {82, "World 19 (unreleased)"},
        {83, "World 20 (unreleased)"},
        {768, "Unknown 768"},
        {769, "Supergirl Red Lantern"},
        {770, "Unknown 770"}};

const std::map<const uint32_t, const char *> s_listTokens = {
        {1000, "Police Car"},
        {1001, "Aerial Squad Car"},
        {1002, "Missile Striker"},
        {1003, "Gravity Sprinter"},
        {1004, "Street Shredder"},
        {1005, "Sky Clobberer"},
        {1006, "Batmobile"},
        {1007, "Batblaster"},
        {1008, "Sonic Batray"},
        {1009, "Benny's Spaceship"},
        {1010, "Lasercraft"},
        {1011, "The Annihilator"},
        {1012, "DeLorean Time Machine"},
        {1013, "Electric Time Machine"},
        {1014, "Ultra Time Machine"},
        {1015, "Hoverboard"},
        {1016, "Cyclone Board"},
        {1017, "Ultimate Hoverjet"},
        {1018, "Eagle Interceptor"},
        {1019, "Eagle Sky Blazer"},
        {1020, "Eagle Swoop Diver"},
        {1021, "Swamp Skimmer"},
        {1022, "Cragger's Fireship"},
        {1023, "Croc Command Sub"},
        {1024, "Cyber-Guard"},
        {1025, "Cyber-Wrecker"},
        {1026, "Laser Robot Walker"},
        {1027, "K-9"},
        {1028, "K-9 Ruff Rover"},
        {1029, "K-9 Laser Cutter"},
        {1030, "TARDIS"},
        {1031, "Laser-Pulse TARDIS"},
        {1032, "Energy-Burst TARDIS"},
        {1033, "Emmet's Excavator"},
        {1034, "Destroy Dozer"},
        {1035, "Destruct-o-Mech"},
        {1036, "Winged Monkey"},
        {1037, "Battle Monkey"},
        {1038, "Commander Monkey"},
        {1039, "Axe Chariot"},
        {1040, "Axe Hurler"},
        {1041, "Soaring Chariot"},
        {1042, "Shelob the Great"},
        {1043, "8-Legged Stalker"},
        {1044, "Poison Slinger"},
        {1045, "Homer's Car"},
        {1047, "The SubmaHomer"},
        {1046, "The Homercraft"},
        {1048, "Taunt-o-Vision"},
        {1050, "The MechaHomer"},
        {1049, "Blast Cam"},
        {1051, "Velociraptor"},
        {1053, "Venom Raptor"},
        {1052, "Spike Attack Raptor"},
        {1054, "Gyrosphere"},
        {1055, "Sonic Beam Gyrosphere"},
        {1056, " Gyrosphere"},
        {1057, "Clown Bike"},
        {1058, "Cannon Bike"},
        {1059, "Anti-Gravity Rocket Bike"},
        {1060, "Mighty Lion Rider"},
        {1061, "Lion Blazer"},
        {1062, "Fire Lion"},
        {1063, "Arrow Launcher"},
        {1064, "Seeking Shooter"},
        {1065, "Triple Ballista"},
        {1066, "Mystery Machine"},
        {1067, "Mystery Tow & Go"},
        {1068, "Mystery Monster"},
        {1069, "Boulder Bomber"},
        {1070, "Boulder Blaster"},
        {1071, "Cyclone Jet"},
        {1072, "Storm Fighter"},
        {1073, "Lightning Jet"},
        {1074, "Electro-Shooter"},
        {1075, "Blade Bike"},
        {1076, "Flight Fire Bike"},
        {1077, "Blades of Fire"},
        {1078, "Samurai Mech"},
        {1079, "Samurai Shooter"},
        {1080, "Soaring Samurai Mech"},
        {1081, "Companion Cube"},
        {1082, "Laser Deflector"},
        {1083, "Gold Heart Emitter"},
        {1084, "Sentry Turret"},
        {1085, "Turret Striker"},
        {1086, "Flight Turret Carrier"},
        {1087, "Scooby Snack"},
        {1088, "Scooby Fire Snack"},
        {1089, "Scooby Ghost Snack"},
        {1090, "Cloud Cuckoo Car"},
        {1091, "X-Stream Soaker"},
        {1092, "Rainbow Cannon"},
        {1093, "Invisible Jet"},
        {1094, "Laser Shooter"},
        {1095, "Torpedo Bomber"},
        {1096, "NinjaCopter"},
        {1097, "Glaciator"},
        {1098, "Freeze Fighter"},
        {1099, "Travelling Time Train"},
        {1100, "Flight Time Train"},
        {1101, "Missile Blast Time Train"},
        {1102, "Aqua Watercraft"},
        {1103, "Seven Seas Speeder"},
        {1104, "Trident of Fire"},
        {1105, "Drill Driver"},
        {1106, "Bane Dig 'n' Drill"},
        {1107, "Bane Drill 'n' Blast"},
        {1108, "Quinn Mobile"},
        {1109, "Quinn Ultra Racer"},
        {1110, "Missile Launcher"},
        {1111, "The Joker's Chopper"},
        {1112, "Mischievous Missile Blaster"},
        {1113, "Lock 'n' Laser Jet"},
        {1114, "Hover Pod"},
        {1115, "Krypton Striker"},
        {1116, "Super Stealth Pod"},
        {1117, "Dalek"},
        {1118, "Fire 'n' Ride Dalek"},
        {1119, "Silver Shooter Dalek"},
        {1120, "Ecto-1"},
        {1121, "Ecto-1 Blaster"},
        {1122, "Ecto-1 Water Diver"},
        {1123, "Ghost Trap"},
        {1124, "Ghost Stun 'n' Trap"},
        {1125, "Proton Zapper"},
        {1126, "Unknown"},
        {1127, "Unknown"},
        {1128, "Unknown"},
        {1129, "Unknown"},
        {1130, "Unknown"},
        {1131, "Unknown"},
        {1132, "Lloyd's Golden Dragon"},
        {1133, "Sword Projector Dragon"},
        {1134, "Unknown"},
        {1135, "Unknown"},
        {1136, "Unknown"},
        {1137, "Unknown"},
        {1138, "Unknown"},
        {1139, "Unknown"},
        {1140, "Unknown"},
        {1141, "Unknown"},
        {1142, "Unknown"},
        {1143, "Unknown"},
        {1144, "Mega Flight Dragon"},
        {1145, "Unknown"},
        {1146, "Unknown"},
        {1147, "Unknown"},
        {1148, "Unknown"},
        {1149, "Unknown"},
        {1150, "Unknown"},
        {1151, "Unknown"},
        {1152, "Unknown"},
        {1153, "Unknown"},
        {1154, "Unknown"},
        {1155, "Flying White Dragon"},
        {1156, "Golden Fire Dragon"},
        {1157, "Ultra Destruction Dragon"},
        {1158, "Arcade Machine"},
        {1159, "8-Bit Shooter"},
        {1160, "The Pixelator"},
        {1161, "G-6155 Spy Hunter"},
        {1162, "Interdiver"},
        {1163, "Aerial Spyhunter"},
        {1164, "Slime Shooter"},
        {1165, "Slime Exploder"},
        {1166, "Slime Streamer"},
        {1167, "Terror Dog"},
        {1168, "Terror Dog Destroyer"},
        {1169, "Soaring Terror Dog"},
        {1170, "Ancient Psychic Tandem War Elephant"},
        {1171, "Cosmic Squid"},
        {1172, "Psychic Submarine"},
        {1173, "BMO"},
        {1174, "DOGMO"},
        {1175, "SNAKEMO"},
        {1176, "Jakemobile"},
        {1177, "Snail Dude Jake"},
        {1178, "Hover Jake"},
        {1179, "Lumpy Car"},
        {1181, "Lumpy Land Whale"},
        {1180, "Lumpy Truck"},
        {1182, "Lunatic Amp"},
        {1183, "Shadow Scorpion"},
        {1184, "Heavy Metal Monster"},
        {1185, "B.A.'s Van"},
        {1186, "Fool Smasher"},
        {1187, "Pain Plane"},
        {1188, "Phone Home"},
        {1189, "Mobile Uplink"},
        {1190, "Super-Charged Satellite"},
        {1191, "Niffler"},
        {1192, "Sinister Scorpion"},
        {1193, "Vicious Vulture"},
        {1194, "Swooping Evil"},
        {1195, "Brutal Bloom"},
        {1196, "Crawling Creeper"},
        {1197, "Ecto-1 (2016)"},
        {1198, "Ectozer"},
        {1199, "PerfEcto"},
        {1200, "Flash 'n' Finish"},
        {1201, "Rampage Record Player"},
        {1202, "Stripe's Throne"},
        {1203, "R.C. Racer"},
        {1204, "Gadget-O-Matic"},
        {1205, "Scarlet Scorpion"},
        {1206, "Hogwarts Express"},
        {1208, "Steam Warrior"},
        {1207, "Soaring Steam Plane"},
        {1209, "Enchanted Car"},
        {1210, "Shark Sub"},
        {1211, "Monstrous Mouth"},
        {1212, "IMF Scrambler"},
        {1213, "Shock Cycle"},
        {1214, "IMF Covert Jet"},
        {1215, "IMF Sports Car"},
        {1216, "IMF Tank"},
        {1217, "IMF Splorer"},
        {1218, "Sonic Speedster"},
        {1219, "Blue Typhoon"},
        {1220, "Moto Bug"},
        {1221, "The Tornado"},
        {1222, "Crabmeat"},
        {1223, "Eggcatcher"},
        {1224, "K.I.T.T."},
        {1225, "Goliath Armored Semi"},
        {1226, "K.I.T.T. Jet"},
        {1227, "Police Helicopter"},
        {1228, "Police Hovercraft"},
        {1229, "Police Plane"},
        {1230, "Bionic Steed"},
        {1231, "Bat-Raptor"},
        {1232, "Ultrabat"},
        {1233, "Batwing"},
        {1234, "The Black Thunder"},
        {1235, "Bat-Tank"},
        {1236, "Skeleton Organ"},
        {1237, "Skeleton Jukebox"},
        {1238, "Skele-Turkey"},
        {1239, "One-Eyed Willy's Pirate Ship"},
        {1240, "Fanged Fortune"},
        {1241, "Inferno Cannon"},
        {1242, "Buckbeak"},
        {1243, "Giant Owl"},
        {1244, "Fierce Falcon"},
        {1245, "Saturn's Sandworm"},
        {1247, "Haunted Vacuum"},
        {1246, "Spooky Spider"},
        {1248, "PPG Smartphone"},
        {1249, "PPG Hotline"},
        {1250, "Powerpuff Mag-Net"},
        {1253, "Mega Blast Bot"},
        {1251, "Ka-Pow Cannon"},
        {1252, "Slammin' Guitar"},
        {1254, "Octi"},
        {1255, "Super Skunk"},
        {1256, "Sonic Squid"},
        {1257, "T-Car"},
        {1258, "T-Forklift"},
        {1259, "T-Plane"},
        {1260, "Spellbook of Azarath"},
        {1261, "Raven Wings"},
        {1262, "Giant Hand"},
        {1263, "Titan Robot"},
        {1264, "T-Rocket"},
        {1265, "Robot Retriever"}};

DimensionsUSBDevice::DimensionsUSBDevice()
    : Device(0x6F0E, 0x4102, 0, 0, 0, 32, 32) {
}

DimensionsUSBDevice::~DimensionsUSBDevice() = default;

bool DimensionsUSBDevice::GetDescriptor(uint8_t descType,
                                        uint8_t descIndex,
                                        uint16_t lang,
                                        uint8_t *buffer,
                                        uint32_t bufferLength) {
    uint8_t configurationDescriptor[0x29];

    uint8_t *currentWritePtr;

    // configuration descriptor
    currentWritePtr                    = configurationDescriptor + 0;
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 2;    // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x29; // wTotalLength
    *(uint8_t *) (currentWritePtr + 3) = 0x00; // wTotalLength
    *(uint8_t *) (currentWritePtr + 4) = 1;    // bNumInterfaces
    *(uint8_t *) (currentWritePtr + 5) = 1;    // bConfigurationValue
    *(uint8_t *) (currentWritePtr + 6) = 0;    // iConfiguration
    *(uint8_t *) (currentWritePtr + 7) = 0x80; // bmAttributes
    *(uint8_t *) (currentWritePtr + 8) = 0xFA; // MaxPower
    currentWritePtr                    = currentWritePtr + 9;
    // interface descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x04; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0;    // bInterfaceNumber
    *(uint8_t *) (currentWritePtr + 3) = 0;    // bAlternateSetting
    *(uint8_t *) (currentWritePtr + 4) = 2;    // bNumEndpoints
    *(uint8_t *) (currentWritePtr + 5) = 3;    // bInterfaceClass
    *(uint8_t *) (currentWritePtr + 6) = 0;    // bInterfaceSubClass
    *(uint8_t *) (currentWritePtr + 7) = 0;    // bInterfaceProtocol
    *(uint8_t *) (currentWritePtr + 8) = 0;    // iInterface
    currentWritePtr                    = currentWritePtr + 9;
    // HID descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x21; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x01; // bcdHID
    *(uint8_t *) (currentWritePtr + 3) = 0x01; // bcdHID
    *(uint8_t *) (currentWritePtr + 4) = 0x00; // bCountryCode
    *(uint8_t *) (currentWritePtr + 5) = 0x01; // bNumDescriptors
    *(uint8_t *) (currentWritePtr + 6) = 0x22; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 7) = 0x1D; // wDescriptorLength
    *(uint8_t *) (currentWritePtr + 8) = 0x00; // wDescriptorLength
    currentWritePtr                    = currentWritePtr + 9;
    // endpoint descriptor 1
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x81; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x20; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;
    // endpoint descriptor 2
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x01; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x20; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;

    memcpy(buffer, configurationDescriptor,
           std::min<uint32_t>(bufferLength, sizeof(configurationDescriptor)));

    return true;
}

bool DimensionsUSBDevice::SetDescriptor(uint8_t descType,
                                        uint8_t descIndex,
                                        uint16_t lang,
                                        uint8_t *buffer,
                                        uint32_t bufferLength) {
    return true;
}

bool DimensionsUSBDevice::GetReport(uint8_t *buffer,
                                    uint32_t bufferLength) {
    return true;
}

bool DimensionsUSBDevice::SetReport(uint8_t *buffer,
                                    uint32_t bufferLength) {
    return true;
}

bool DimensionsUSBDevice::GetIdle(uint8_t ifIndex,
                                  uint8_t reportId,
                                  uint8_t *duration) {
    return true;
}

bool DimensionsUSBDevice::SetIdle(uint8_t ifIndex,
                                  uint8_t reportId,
                                  uint8_t duration) {
    return true;
}

bool DimensionsUSBDevice::GetProtocol(uint8_t ifIndex,
                                      uint8_t *protocol) {
    return true;
}

bool DimensionsUSBDevice::SetProtocol(uint8_t ifIndex,
                                      uint8_t protocol) {
    return true;
}

bool DimensionsUSBDevice::Read(uint8_t *buffer,
                               uint32_t bufferLength) {
    memcpy(buffer, g_dimensionstoypad.GetStatus().data(), bufferLength);
    return true;
}

bool DimensionsUSBDevice::Write(uint8_t *buffer,
                                uint32_t bufferLength) {
    if (bufferLength != 32)
        return false;

    g_dimensionstoypad.SendCommand(std::span<const uint8_t, 32>{buffer, 32});
    return true;
}

std::array<uint8_t, 32> DimensionsToypad::GetStatus() {
    std::array<uint8_t, 32> response = {};

    bool responded = false;
    do {
        if (!m_queries.empty()) {
            response = m_queries.front();
            m_queries.pop();
            responded           = true;
            m_wasLastRespFigure = false;
            m_noResponseCount   = 0;
        } else if (!m_figureAddedRemovedResponses.empty() && m_isAwake && (!m_wasLastRespFigure || m_noResponseCount >= 10)) {
            std::lock_guard lock(m_dimensionsMutex);
            response = m_figureAddedRemovedResponses.front();
            m_figureAddedRemovedResponses.pop();
            responded           = true;
            m_wasLastRespFigure = true;
            m_noResponseCount   = 0;
        } else {
            m_noResponseCount++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } while (responded == false);
    return response;
}

void DimensionsToypad::SendCommand(std::span<const uint8_t, 32> buf) {
    const uint8_t command  = buf[2];
    const uint8_t sequence = buf[3];

    std::array<uint8_t, 32> q_result{};

    switch (command) {
        case 0xB0: // Wake
        {
            // Consistent device response to the wake command
            q_result = {0x55, 0x0e, 0x01, 0x28, 0x63, 0x29,
                        0x20, 0x4c, 0x45, 0x47, 0x4f, 0x20,
                        0x32, 0x30, 0x31, 0x34, 0x46};
            break;
        }
        case 0xB1: // Seed
        {
            // Initialise a random number generator using the seed provided
            g_dimensionstoypad.GenerateRandomNumber(std::span<const uint8_t, 8>{buf.begin() + 4, 8}, sequence, q_result);
            break;
        }
        case 0xB3: // Challenge
        {
            // Get the next number in the sequence based on the RNG from 0xB1 command
            g_dimensionstoypad.GetChallengeResponse(std::span<const uint8_t, 8>{buf.begin() + 4, 8}, sequence, q_result);
            break;
        }
        case 0xC0: // Color
        {
            g_dimensionstoypad.SetPadColor(std::span<const uint8_t, 4>{buf.begin() + 4, 4}, sequence, q_result);
            break;
        }
        case 0xC2: // Fade
        {
            g_dimensionstoypad.SetPadFade(std::span<const uint8_t, 6>{buf.begin() + 4, 6}, sequence, q_result);
            break;
        }
        case 0xC3: // Flash
        {
            g_dimensionstoypad.SetPadFlash(std::span<const uint8_t, 7>{buf.begin() + 4, 7}, sequence, q_result);
            break;
        }
        case 0xC4: // Fade Random
        {
            g_dimensionstoypad.SetFadeRandom(std::span<const uint8_t, 3>{buf.begin() + 4, 3}, sequence, q_result);
            break;
        }
        case 0xC6: // Fade All
        {
            g_dimensionstoypad.SetFadeAll(std::span<const uint8_t, 18>{buf.begin() + 4, 18}, sequence, q_result);
            break;
        }
        case 0xC7: // Flash All
        {
            g_dimensionstoypad.SetFlashAll(std::span<const uint8_t, 21>{buf.begin() + 4, 21}, sequence, q_result);
            break;
        }
        case 0xC8: // Color All
        {
            g_dimensionstoypad.SetColorAll(std::span<const uint8_t, 12>{buf.begin() + 4, 12}, sequence, q_result);
            break;
        }
        case 0xD2: // Read
        {
            // Read 4 pages from the figure at index (buf[4]), starting with page buf[5]
            g_dimensionstoypad.QueryBlock(buf[4], buf[5], q_result, sequence);
            break;
        }
        case 0xD3: // Write
        {
            // Write 4 bytes to page buf[5] to the figure at index buf[4]
            g_dimensionstoypad.WriteBlock(buf[4], buf[5], std::span<const uint8_t, 4>{buf.begin() + 6, 4}, q_result, sequence);
            break;
        }
        case 0xD4: // Model
        {
            // Get the model id of the figure at index buf[4]
            g_dimensionstoypad.GetModel(std::span<const uint8_t, 8>{buf.begin() + 4, 8}, sequence, q_result);
            break;
        }
        case 0xC1: // Get Pad Color (debug only)
        case 0xD0: // Tag List
        case 0xE1: // PWD
        case 0xE5: // Active
        case 0xFF: // LEDS Query
        {
            // Further investigation required
            DEBUG_FUNCTION_LINE_ERR("Unimplemented LD Function: %02X", command);
            break;
        }
        default: {
            DEBUG_FUNCTION_LINE_ERR("Unknown LD Function: %02X", command);
            break;
        }
    }

    m_queries.push(q_result);
}

void DimensionsToypad::GenerateRandomNumber(std::span<const uint8_t, 8> buf, uint8_t sequence,
                                            std::array<uint8_t, 32> &replyBuf) {
    // Decrypt payload into an 8 byte array
    std::array<uint8_t, 8> value = Decrypt(buf, std::nullopt);
    // Seed is the first 4 bytes (little endian) of the decrypted payload
    uint32_t seed = uint32_t(value[0]) | (uint32_t(value[1]) << 8) | (uint32_t(value[2]) << 16) | (uint32_t(value[3]) << 24);
    // Confirmation is the second 4 bytes (big endian) of the decrypted payload
    // Initialize rng using the seed from decrypted payload
    InitializeRNG(seed);
    // Encrypt 8 bytes, first 4 bytes is the decrypted confirmation from payload, 2nd 4 bytes are blank
    std::array<uint8_t, 8> valueToEncrypt = {value[4], value[5], value[6], value[7], 0, 0, 0, 0};
    std::array<uint8_t, 8> encrypted      = Encrypt(valueToEncrypt, std::nullopt);
    replyBuf[0]                           = 0x55;
    replyBuf[1]                           = 0x09;
    replyBuf[2]                           = sequence;
    // Copy encrypted value to response data
    memcpy(&replyBuf[3], encrypted.data(), encrypted.size());
    replyBuf[11] = GenerateChecksum(replyBuf, 11);
}

void DimensionsToypad::GetChallengeResponse(std::span<const uint8_t, 8> buf, uint8_t sequence,
                                            std::array<uint8_t, 32> &replyBuf) {
    // Decrypt payload into an 8 byte array
    std::array<uint8_t, 8> value = Decrypt(buf, std::nullopt);
    // Confirmation is the first 4 bytes of the decrypted payload
    // Generate next random number based on RNG
    uint32_t nextRandom = GetNext();
    // Encrypt an 8 byte array, first 4 bytes are the next random number (little endian)
    // followed by the confirmation from the decrypted payload
    std::array<uint8_t, 8> valueToEncrypt = {uint8_t(nextRandom & 0xFF), uint8_t((nextRandom >> 8) & 0xFF),
                                             uint8_t((nextRandom >> 16) & 0xFF), uint8_t((nextRandom >> 24) & 0xFF),
                                             value[0], value[1], value[2], value[3]};
    std::array<uint8_t, 8> encrypted      = Encrypt(valueToEncrypt, std::nullopt);
    replyBuf[0]                           = 0x55;
    replyBuf[1]                           = 0x09;
    replyBuf[2]                           = sequence;
    // Copy encrypted value to response data
    memcpy(&replyBuf[3], encrypted.data(), encrypted.size());
    replyBuf[11] = GenerateChecksum(replyBuf, 11);

    if (!m_isAwake)
        m_isAwake = true;
}

void DimensionsToypad::SetPadColor(std::span<const uint8_t, 4> buf, uint8_t sequence,
                                   std::array<uint8_t, 32> &replyBuf) {
    uint8_t pad = buf[0];
    if (pad == 0) {
        m_colorTop.red         = buf[1];
        m_colorLeft.red        = buf[1];
        m_colorRight.red       = buf[1];
        m_colorTop.green       = buf[2];
        m_colorLeft.green      = buf[2];
        m_colorRight.green     = buf[2];
        m_colorTop.blue        = buf[3];
        m_colorLeft.blue       = buf[3];
        m_colorRight.blue      = buf[3];
        m_colorTop.colorType   = DimensionsColorType::COLOR;
        m_colorLeft.colorType  = DimensionsColorType::COLOR;
        m_colorRight.colorType = DimensionsColorType::COLOR;
    } else if (pad == 1) {
        m_colorTop.red         = buf[1];
        m_colorTop.green       = buf[2];
        m_colorTop.blue        = buf[3];
        m_colorTop.colorType   = DimensionsColorType::COLOR;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 2) {
        m_colorLeft.red        = buf[1];
        m_colorLeft.green      = buf[2];
        m_colorLeft.blue       = buf[3];
        m_colorLeft.colorType  = DimensionsColorType::COLOR;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 3) {
        m_colorRight.red       = buf[1];
        m_colorRight.green     = buf[2];
        m_colorRight.blue      = buf[3];
        m_colorRight.colorType = DimensionsColorType::COLOR;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
    }
    replyBuf    = {0x55, 0x01, sequence};
    replyBuf[3] = GenerateChecksum(replyBuf, 3);
}

void DimensionsToypad::SetPadFade(std::span<const uint8_t, 6> buf, uint8_t sequence,
                                  std::array<uint8_t, 32> &replyBuf) {
    uint8_t pad = buf[0];
    if (pad == 0) {
        m_colorTop.speed       = buf[1];
        m_colorLeft.speed      = buf[1];
        m_colorRight.speed     = buf[1];
        m_colorTop.cycles      = buf[2];
        m_colorLeft.cycles     = buf[2];
        m_colorRight.cycles    = buf[2];
        m_colorTop.red         = buf[3];
        m_colorLeft.red        = buf[3];
        m_colorRight.red       = buf[3];
        m_colorTop.green       = buf[4];
        m_colorLeft.green      = buf[4];
        m_colorRight.green     = buf[4];
        m_colorTop.blue        = buf[5];
        m_colorLeft.blue       = buf[5];
        m_colorRight.blue      = buf[5];
        m_colorTop.colorType   = DimensionsColorType::FADE;
        m_colorLeft.colorType  = DimensionsColorType::FADE;
        m_colorRight.colorType = DimensionsColorType::FADE;
    } else if (pad == 1) {
        m_colorTop.speed       = buf[1];
        m_colorTop.cycles      = buf[2];
        m_colorTop.red         = buf[3];
        m_colorTop.green       = buf[4];
        m_colorTop.blue        = buf[5];
        m_colorTop.colorType   = DimensionsColorType::FADE;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 2) {
        m_colorLeft.speed      = buf[1];
        m_colorLeft.cycles     = buf[2];
        m_colorLeft.red        = buf[3];
        m_colorLeft.green      = buf[4];
        m_colorLeft.blue       = buf[5];
        m_colorLeft.colorType  = DimensionsColorType::FADE;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 3) {
        m_colorRight.speed     = buf[1];
        m_colorRight.cycles    = buf[2];
        m_colorRight.red       = buf[3];
        m_colorRight.green     = buf[4];
        m_colorRight.blue      = buf[5];
        m_colorRight.colorType = DimensionsColorType::FADE;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
    }
    replyBuf    = {0x55, 0x01, sequence};
    replyBuf[3] = GenerateChecksum(replyBuf, 3);
}

void DimensionsToypad::SetPadFlash(std::span<const uint8_t, 7> buf, uint8_t sequence,
                                   std::array<uint8_t, 32> &replyBuf) {
    uint8_t pad = buf[0];
    if (pad == 0) {
        m_colorTop.colorDuration   = buf[1];
        m_colorLeft.colorDuration  = buf[1];
        m_colorRight.colorDuration = buf[1];
        m_colorTop.whiteDuration   = buf[2];
        m_colorLeft.whiteDuration  = buf[2];
        m_colorRight.whiteDuration = buf[2];
        m_colorTop.cycles          = buf[3];
        m_colorLeft.cycles         = buf[3];
        m_colorRight.cycles        = buf[3];
        m_colorTop.red             = buf[4];
        m_colorLeft.red            = buf[4];
        m_colorRight.red           = buf[4];
        m_colorTop.green           = buf[5];
        m_colorLeft.green          = buf[5];
        m_colorRight.green         = buf[5];
        m_colorTop.blue            = buf[6];
        m_colorLeft.blue           = buf[6];
        m_colorRight.blue          = buf[6];
        m_colorTop.colorType       = DimensionsColorType::FLASH;
        m_colorLeft.colorType      = DimensionsColorType::FLASH;
        m_colorRight.colorType     = DimensionsColorType::FLASH;
    } else if (pad == 1) {
        m_colorTop.colorDuration = buf[1];
        m_colorTop.whiteDuration = buf[2];
        m_colorTop.cycles        = buf[3];
        m_colorTop.red           = buf[4];
        m_colorTop.green         = buf[5];
        m_colorTop.blue          = buf[6];
        m_colorTop.colorType     = DimensionsColorType::FLASH;
        m_colorLeft.colorType    = DimensionsColorType::NONE;
        m_colorRight.colorType   = DimensionsColorType::NONE;
    } else if (pad == 2) {
        m_colorLeft.colorDuration = buf[1];
        m_colorLeft.whiteDuration = buf[2];
        m_colorLeft.cycles        = buf[3];
        m_colorLeft.red           = buf[4];
        m_colorLeft.green         = buf[5];
        m_colorLeft.blue          = buf[6];
        m_colorLeft.colorType     = DimensionsColorType::FLASH;
        m_colorTop.colorType      = DimensionsColorType::NONE;
        m_colorRight.colorType    = DimensionsColorType::NONE;
    } else if (pad == 3) {
        m_colorRight.colorDuration = buf[1];
        m_colorRight.whiteDuration = buf[2];
        m_colorRight.cycles        = buf[3];
        m_colorRight.red           = buf[4];
        m_colorRight.green         = buf[5];
        m_colorRight.blue          = buf[6];
        m_colorRight.colorType     = DimensionsColorType::FLASH;
        m_colorTop.colorType       = DimensionsColorType::NONE;
        m_colorLeft.colorType      = DimensionsColorType::NONE;
    }
    replyBuf    = {0x55, 0x01, sequence};
    replyBuf[3] = GenerateChecksum(replyBuf, 3);
}

void DimensionsToypad::SetFadeRandom(std::span<const uint8_t, 3> buf, uint8_t sequence,
                                     std::array<uint8_t, 32> &replyBuf) {
    uint8_t pad = buf[0];
    if (pad == 0) {
        m_colorTop.speed       = buf[1];
        m_colorLeft.speed      = buf[1];
        m_colorRight.speed     = buf[1];
        m_colorTop.cycles      = buf[2];
        m_colorLeft.cycles     = buf[2];
        m_colorRight.cycles    = buf[2];
        m_colorTop.red         = uint8_t(rand() % 255);
        m_colorLeft.red        = uint8_t(rand() % 255);
        m_colorRight.red       = uint8_t(rand() % 255);
        m_colorTop.green       = uint8_t(rand() % 255);
        m_colorLeft.green      = uint8_t(rand() % 255);
        m_colorRight.green     = uint8_t(rand() % 255);
        m_colorTop.blue        = uint8_t(rand() % 255);
        m_colorLeft.blue       = uint8_t(rand() % 255);
        m_colorRight.blue      = uint8_t(rand() % 255);
        m_colorTop.colorType   = DimensionsColorType::FADE;
        m_colorLeft.colorType  = DimensionsColorType::FADE;
        m_colorRight.colorType = DimensionsColorType::FADE;
    } else if (pad == 1) {
        m_colorTop.speed       = buf[1];
        m_colorTop.cycles      = buf[2];
        m_colorTop.red         = uint8_t(rand() % 255);
        m_colorTop.green       = uint8_t(rand() % 255);
        m_colorTop.blue        = uint8_t(rand() % 255);
        m_colorTop.colorType   = DimensionsColorType::FADE;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 2) {
        m_colorLeft.speed      = buf[1];
        m_colorLeft.cycles     = buf[2];
        m_colorLeft.red        = uint8_t(rand() % 255);
        m_colorLeft.green      = uint8_t(rand() % 255);
        m_colorLeft.blue       = uint8_t(rand() % 255);
        m_colorLeft.colorType  = DimensionsColorType::FADE;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorRight.colorType = DimensionsColorType::NONE;
    } else if (pad == 3) {
        m_colorRight.speed     = buf[1];
        m_colorRight.cycles    = buf[2];
        m_colorRight.red       = uint8_t(rand() % 255);
        m_colorRight.green     = uint8_t(rand() % 255);
        m_colorRight.blue      = uint8_t(rand() % 255);
        m_colorRight.colorType = DimensionsColorType::FADE;
        m_colorTop.colorType   = DimensionsColorType::NONE;
        m_colorLeft.colorType  = DimensionsColorType::NONE;
    }
}

void DimensionsToypad::SetFadeAll(std::span<const uint8_t, 18> buf, uint8_t sequence,
                                  std::array<uint8_t, 32> &replyBuf) {
    m_colorTop.speed       = buf[1];
    m_colorTop.cycles      = buf[2];
    m_colorTop.red         = buf[3];
    m_colorTop.green       = buf[4];
    m_colorTop.blue        = buf[5];
    m_colorLeft.speed      = buf[7];
    m_colorLeft.cycles     = buf[8];
    m_colorLeft.red        = buf[9];
    m_colorLeft.green      = buf[10];
    m_colorLeft.blue       = buf[11];
    m_colorRight.speed     = buf[13];
    m_colorRight.cycles    = buf[14];
    m_colorRight.red       = buf[15];
    m_colorRight.green     = buf[16];
    m_colorRight.blue      = buf[17];
    m_colorTop.colorType   = DimensionsColorType::FADE;
    m_colorLeft.colorType  = DimensionsColorType::FADE;
    m_colorRight.colorType = DimensionsColorType::FADE;
    replyBuf               = {0x55, 0x01, sequence};
    replyBuf[3]            = GenerateChecksum(replyBuf, 3);
}


void DimensionsToypad::SetFlashAll(std::span<const uint8_t, 21> buf, uint8_t sequence,
                                   std::array<uint8_t, 32> &replyBuf) {
    m_colorTop.colorDuration   = buf[1];
    m_colorTop.whiteDuration   = buf[2];
    m_colorTop.cycles          = buf[3];
    m_colorTop.red             = buf[4];
    m_colorTop.green           = buf[5];
    m_colorTop.blue            = buf[6];
    m_colorLeft.colorDuration  = buf[8];
    m_colorLeft.whiteDuration  = buf[9];
    m_colorLeft.cycles         = buf[10];
    m_colorLeft.red            = buf[11];
    m_colorLeft.green          = buf[12];
    m_colorLeft.blue           = buf[13];
    m_colorRight.colorDuration = buf[15];
    m_colorRight.whiteDuration = buf[16];
    m_colorRight.cycles        = buf[17];
    m_colorRight.red           = buf[18];
    m_colorRight.green         = buf[19];
    m_colorRight.blue          = buf[20];
    m_colorTop.colorType       = DimensionsColorType::FLASH;
    m_colorLeft.colorType      = DimensionsColorType::FLASH;
    m_colorRight.colorType     = DimensionsColorType::FLASH;
    replyBuf                   = {0x55, 0x01, sequence};
    replyBuf[3]                = GenerateChecksum(replyBuf, 3);
}

void DimensionsToypad::SetColorAll(std::span<const uint8_t, 12> buf, uint8_t sequence,
                                   std::array<uint8_t, 32> &replyBuf) {
    m_colorTop.red         = buf[1];
    m_colorTop.green       = buf[2];
    m_colorTop.blue        = buf[3];
    m_colorLeft.red        = buf[5];
    m_colorRight.red       = buf[6];
    m_colorLeft.green      = buf[7];
    m_colorRight.green     = buf[9];
    m_colorLeft.blue       = buf[10];
    m_colorRight.blue      = buf[11];
    m_colorTop.colorType   = DimensionsColorType::COLOR;
    m_colorLeft.colorType  = DimensionsColorType::COLOR;
    m_colorRight.colorType = DimensionsColorType::COLOR;
    replyBuf               = {0x55, 0x01, sequence};
    replyBuf[3]            = GenerateChecksum(replyBuf, 3);
}

void DimensionsToypad::QueryBlock(uint8_t index, uint8_t page,
                                  std::array<uint8_t, 32> &replyBuf,
                                  uint8_t sequence) {
    std::lock_guard lock(m_dimensionsMutex);

    replyBuf[0] = 0x55;
    replyBuf[1] = 0x12;
    replyBuf[2] = sequence;
    replyBuf[3] = 0x00;

    // Index from game begins at 1 rather than 0, so minus 1 here
    if (const uint8_t figureIndex = index - 1; figureIndex < 7) {
        const DimensionsMini &figure = GetFigureByIndex(figureIndex);

        // Query 4 pages of 4 bytes from the figure, copy this to the response
        if (figure.index != 255 && (4 * page) < ((0x2D * 4) - 16)) {
            std::memcpy(&replyBuf[4], figure.data.data() + (4 * page), 16);
        }
    }
    replyBuf[20] = GenerateChecksum(replyBuf, 20);
}

void DimensionsToypad::WriteBlock(uint8_t index, uint8_t page, std::span<const uint8_t, 4> toWriteBuf,
                                  std::array<uint8_t, 32> &replyBuf, uint8_t sequence) {
    std::lock_guard lock(m_dimensionsMutex);

    replyBuf[0] = 0x55;
    replyBuf[1] = 0x02;
    replyBuf[2] = sequence;
    replyBuf[3] = 0x00;

    // Index from game begins at 1 rather than 0, so minus 1 here
    if (const uint8_t figureIndex = index - 1; figureIndex < 7) {
        DimensionsMini &figure = GetFigureByIndex(figureIndex);

        // Copy 4 bytes to the page on the figure requested by the game
        if (figure.index != 255 && page < 0x2D) {
            // Id is written to page 36
            if (page == 36) {
                figure.id = uint32_t(toWriteBuf[0]) | (uint32_t(toWriteBuf[1]) << 8) | (uint32_t(toWriteBuf[2]) << 16) | (uint32_t(toWriteBuf[3]) << 24);
            }
            std::memcpy(figure.data.data() + (page * 4), toWriteBuf.data(), 4);
            figure.Save();
        }
    }
    replyBuf[4] = GenerateChecksum(replyBuf, 4);
}

void DimensionsToypad::GetModel(std::span<const uint8_t, 8> buf, uint8_t sequence,
                                std::array<uint8_t, 32> &replyBuf) {
    // Decrypt payload to 8 byte array, byte 1 is the index, 4-7 are the confirmation
    std::array<uint8_t, 8> value = Decrypt(buf, std::nullopt);
    uint8_t index                = value[0];
    // Response is the figure's id (little endian) followed by the confirmation from payload
    // Index from game begins at 1 rather than 0, so minus 1 here
    std::array<uint8_t, 8> valueToEncrypt = {};
    if (const uint8_t figureIndex = index - 1; figureIndex < 7) {
        const DimensionsMini &figure = GetFigureByIndex(figureIndex);
        valueToEncrypt               = {uint8_t(figure.id & 0xFF), uint8_t((figure.id >> 8) & 0xFF),
                                        uint8_t((figure.id >> 16) & 0xFF), uint8_t((figure.id >> 24) & 0xFF),
                                        value[4], value[5], value[6], value[7]};
    }
    std::array<uint8_t, 8> encrypted = Encrypt(valueToEncrypt, std::nullopt);
    replyBuf[0]                      = 0x55;
    replyBuf[1]                      = 0x0a;
    replyBuf[2]                      = sequence;
    replyBuf[3]                      = 0x00;
    memcpy(&replyBuf[4], encrypted.data(), encrypted.size());
    replyBuf[12] = GenerateChecksum(replyBuf, 12);
}

bool DimensionsToypad::RemoveFigure(uint8_t pad, uint8_t index, bool fullRemove) {
    std::lock_guard lock(m_dimensionsMutex);

    DimensionsMini &figure = GetFigureByIndex(index);
    if (figure.index == 255)
        return false;

    // When a figure is removed from the toypad, respond to the game with the pad they were removed from, their index,
    // the direction (0x01 in byte 6 for removed) and their UID
    if (fullRemove) {
        std::array<uint8_t, 32> figureChangeResponse = {0x56, 0x0b, figure.pad, 0x00, figure.index, 0x01,
                                                        figure.data[0], figure.data[1], figure.data[2],
                                                        figure.data[4], figure.data[5], figure.data[6], figure.data[7]};
        figureChangeResponse[13]                     = GenerateChecksum(figureChangeResponse, 13);
        m_figureAddedRemovedResponses.push(figureChangeResponse);
        figure.Save();
        figure.filePath = "";
    }

    figure.index = 255;
    figure.pad   = 255;
    figure.id    = 0;

    return true;
}

bool DimensionsToypad::TempRemove(uint8_t index) {
    std::lock_guard lock(m_dimensionsMutex);

    DimensionsMini &figure = GetFigureByIndex(index);
    if (figure.index == 255)
        return false;

    // Send a response to the game that the figure has been "Picked up" from existing slot,
    // until either the movement is cancelled, or user chooses a space to move to
    std::array<uint8_t, 32> figureChangeResponse = {0x56, 0x0b, figure.pad, 0x00, figure.index, 0x01,
                                                    figure.data[0], figure.data[1], figure.data[2],
                                                    figure.data[4], figure.data[5], figure.data[6], figure.data[7]};

    figureChangeResponse[13] = GenerateChecksum(figureChangeResponse, 13);
    m_figureAddedRemovedResponses.push(figureChangeResponse);
    return true;
}

bool DimensionsToypad::CancelRemove(uint8_t index) {
    std::lock_guard lock(m_dimensionsMutex);

    DimensionsMini &figure = GetFigureByIndex(index);
    if (figure.index == 255)
        return false;

    // Cancel the previous movement of the figure
    std::array<uint8_t, 32> figureChangeResponse = {0x56, 0x0b, figure.pad, 0x00, figure.index, 0x00,
                                                    figure.data[0], figure.data[1], figure.data[2],
                                                    figure.data[4], figure.data[5], figure.data[6], figure.data[7]};

    figureChangeResponse[13] = GenerateChecksum(figureChangeResponse, 13);
    m_figureAddedRemovedResponses.push(figureChangeResponse);

    return true;
}

uint32_t DimensionsToypad::LoadFigure(const std::array<uint8_t, 0x2D * 0x04> &buf, std::string file, uint8_t pad, uint8_t index) {
    std::lock_guard lock(m_dimensionsMutex);

    const uint32_t id = GetFigureId(buf);

    DimensionsMini &figure = GetFigureByIndex(index);
    figure.filePath        = file;
    figure.id              = id;
    figure.pad             = pad;
    figure.index           = index + 1;
    figure.data            = buf;
    // When a figure is added to the toypad, respond to the game with the pad they were added to, their index,
    // the direction (0x00 in byte 6 for added) and their UID
    std::array<uint8_t, 32> figureChangeResponse = {0x56, 0x0b, figure.pad, 0x00, figure.index, 0x00, buf[0], buf[1], buf[2], buf[4], buf[5], buf[6], buf[7]};
    figureChangeResponse[13]                     = GenerateChecksum(figureChangeResponse, 13);
    m_figureAddedRemovedResponses.push(figureChangeResponse);

    return id;
}

bool DimensionsToypad::CreateFigure(std::string pathName, uint32_t id) {
    std::ofstream dimFile(pathName.c_str(), std::ios::binary);
    if (!dimFile) {
        return false;
    }

    std::array<uint8_t, 0x2D * 0x04> fileData{};
    RandomUID(fileData);
    fileData[3] = id & 0xFF;

    // Only characters are created with their ID encrypted and stored in pages 36 and 37,
    // as well as a password stored in page 43. Blank tags have their information populated
    // by the game when it calls the write_block command.
    if (id != 0 && id < 1000) {
        const std::array<uint8_t, 16> figureKey = GenerateFigureKey(fileData);

        std::array<uint8_t, 8> valueToEncrypt = {uint8_t(id & 0xFF), uint8_t((id >> 8) & 0xFF), uint8_t((id >> 16) & 0xFF), uint8_t((id >> 24) & 0xFF),
                                                 uint8_t(id & 0xFF), uint8_t((id >> 8) & 0xFF), uint8_t((id >> 16) & 0xFF), uint8_t((id >> 24) & 0xFF)};

        std::array<uint8_t, 8> encrypted = Encrypt(valueToEncrypt, figureKey);

        std::memcpy(&fileData[36 * 4], &encrypted[0], 4);
        std::memcpy(&fileData[37 * 4], &encrypted[4], 4);

        std::memcpy(&fileData[43 * 4], PWDGenerate(fileData).data(), 4);
    } else {
        // Page 36 stores gadget/vehicle IDs as little endian
        fileData[36 * 4]       = id & 0xFF;
        fileData[(36 * 4) + 1] = (id >> 8) & 0xFF;
        fileData[(36 * 4) + 2] = (id >> 16) & 0xFF;
        fileData[(36 * 4) + 3] = (id >> 24) & 0xFF;
        // Page 38 is used as verification for blank tags
        fileData[(38 * 4) + 1] = 1;
    }

    dimFile.write((char *) fileData.data(), fileData.size());

    dimFile.close();

    return true;
}

bool DimensionsToypad::MoveFigure(uint8_t pad, uint8_t index, uint8_t oldPad, uint8_t oldIndex) {
    if (oldIndex == index) {
        // Don't bother removing and loading again, just send response to the game
        CancelRemove(index);
        return true;
    }

    // When moving figures between spaces on the toypad, remove any figure from the space they are moving to,
    // then remove them from their current space, then load them to the space they are moving to
    RemoveFigure(pad, index, true);

    DimensionsMini &figure                          = GetFigureByIndex(oldIndex);
    const std::array<uint8_t, DIM_FIGURE_SIZE> data = figure.data;
    std::string inFile                              = figure.filePath;

    RemoveFigure(oldPad, oldIndex, false);

    LoadFigure(data, inFile, pad, index);

    return true;
}

void DimensionsToypad::InitializeRNG(uint32_t seed) {
    m_randomA = 0xF1EA5EED;
    m_randomB = seed;
    m_randomC = seed;
    m_randomD = seed;

    for (int i = 0; i < 42; i++) {
        GetNext();
    }
}

uint32_t DimensionsToypad::GetNext() {
    uint32_t e = m_randomA - std::rotl(m_randomB, 21);
    m_randomA  = m_randomB ^ std::rotl(m_randomC, 19);
    m_randomB  = m_randomC + std::rotl(m_randomD, 6);
    m_randomC  = m_randomD + e;
    m_randomD  = e + m_randomA;
    return m_randomD;
}

std::array<uint8_t, 8> DimensionsToypad::Decrypt(std::span<const uint8_t, 8> buf, std::optional<std::array<uint8_t, 16>> key) {
    // Value to decrypt is separated in to two little endian 32 bit unsigned integers
    uint32_t dataOne = uint32_t(buf[0]) | (uint32_t(buf[1]) << 8) | (uint32_t(buf[2]) << 16) | (uint32_t(buf[3]) << 24);
    uint32_t dataTwo = uint32_t(buf[4]) | (uint32_t(buf[5]) << 8) | (uint32_t(buf[6]) << 16) | (uint32_t(buf[7]) << 24);

    // Use the key as 4 32 bit little endian unsigned integers
    uint32_t keyOne;
    uint32_t keyTwo;
    uint32_t keyThree;
    uint32_t keyFour;

    if (key) {
        keyOne   = uint32_t(key.value()[0]) | (uint32_t(key.value()[1]) << 8) | (uint32_t(key.value()[2]) << 16) | (uint32_t(key.value()[3]) << 24);
        keyTwo   = uint32_t(key.value()[4]) | (uint32_t(key.value()[5]) << 8) | (uint32_t(key.value()[6]) << 16) | (uint32_t(key.value()[7]) << 24);
        keyThree = uint32_t(key.value()[8]) | (uint32_t(key.value()[9]) << 8) | (uint32_t(key.value()[10]) << 16) | (uint32_t(key.value()[11]) << 24);
        keyFour  = uint32_t(key.value()[12]) | (uint32_t(key.value()[13]) << 8) | (uint32_t(key.value()[14]) << 16) | (uint32_t(key.value()[15]) << 24);
    } else {
        keyOne   = uint32_t(COMMAND_KEY[0]) | (uint32_t(COMMAND_KEY[1]) << 8) | (uint32_t(COMMAND_KEY[2]) << 16) | (uint32_t(COMMAND_KEY[3]) << 24);
        keyTwo   = uint32_t(COMMAND_KEY[4]) | (uint32_t(COMMAND_KEY[5]) << 8) | (uint32_t(COMMAND_KEY[6]) << 16) | (uint32_t(COMMAND_KEY[7]) << 24);
        keyThree = uint32_t(COMMAND_KEY[8]) | (uint32_t(COMMAND_KEY[9]) << 8) | (uint32_t(COMMAND_KEY[10]) << 16) | (uint32_t(COMMAND_KEY[11]) << 24);
        keyFour  = uint32_t(COMMAND_KEY[12]) | (uint32_t(COMMAND_KEY[13]) << 8) | (uint32_t(COMMAND_KEY[14]) << 16) | (uint32_t(COMMAND_KEY[15]) << 24);
    }

    uint32_t sum   = 0xC6EF3720;
    uint32_t delta = 0x9E3779B9;

    for (int i = 0; i < 32; i++) {
        dataTwo -= (((dataOne << 4) + keyThree) ^ (dataOne + sum) ^ ((dataOne >> 5) + keyFour));
        dataOne -= (((dataTwo << 4) + keyOne) ^ (dataTwo + sum) ^ ((dataTwo >> 5) + keyTwo));
        sum -= delta;
    }

    std::array<uint8_t, 8> decrypted = {uint8_t(dataOne & 0xFF), uint8_t((dataOne >> 8) & 0xFF),
                                        uint8_t((dataOne >> 16) & 0xFF), uint8_t((dataOne >> 24) & 0xFF),
                                        uint8_t(dataTwo & 0xFF), uint8_t((dataTwo >> 8) & 0xFF),
                                        uint8_t((dataTwo >> 16) & 0xFF), uint8_t((dataTwo >> 24) & 0xFF)};
    return decrypted;
}

std::array<uint8_t, 8> DimensionsToypad::Encrypt(std::span<const uint8_t, 8> buf, std::optional<std::array<uint8_t, 16>> key) {
    // Value to encrypt is separated in to two little endian 32 bit unsigned integers
    uint32_t dataOne = uint32_t(buf[0]) | (uint32_t(buf[1]) << 8) | (uint32_t(buf[2]) << 16) | (uint32_t(buf[3]) << 24);
    uint32_t dataTwo = uint32_t(buf[4]) | (uint32_t(buf[5]) << 8) | (uint32_t(buf[6]) << 16) | (uint32_t(buf[7]) << 24);

    // Use the key as 4 32 bit little endian unsigned integers
    uint32_t keyOne;
    uint32_t keyTwo;
    uint32_t keyThree;
    uint32_t keyFour;

    if (key) {
        keyOne   = uint32_t(key.value()[0]) | (uint32_t(key.value()[1]) << 8) | (uint32_t(key.value()[2]) << 16) | (uint32_t(key.value()[3]) << 24);
        keyTwo   = uint32_t(key.value()[4]) | (uint32_t(key.value()[5]) << 8) | (uint32_t(key.value()[6]) << 16) | (uint32_t(key.value()[7]) << 24);
        keyThree = uint32_t(key.value()[8]) | (uint32_t(key.value()[9]) << 8) | (uint32_t(key.value()[10]) << 16) | (uint32_t(key.value()[11]) << 24);
        keyFour  = uint32_t(key.value()[12]) | (uint32_t(key.value()[13]) << 8) | (uint32_t(key.value()[14]) << 16) | (uint32_t(key.value()[15]) << 24);
    } else {
        keyOne   = uint32_t(COMMAND_KEY[0]) | (uint32_t(COMMAND_KEY[1]) << 8) | (uint32_t(COMMAND_KEY[2]) << 16) | (uint32_t(COMMAND_KEY[3]) << 24);
        keyTwo   = uint32_t(COMMAND_KEY[4]) | (uint32_t(COMMAND_KEY[5]) << 8) | (uint32_t(COMMAND_KEY[6]) << 16) | (uint32_t(COMMAND_KEY[7]) << 24);
        keyThree = uint32_t(COMMAND_KEY[8]) | (uint32_t(COMMAND_KEY[9]) << 8) | (uint32_t(COMMAND_KEY[10]) << 16) | (uint32_t(COMMAND_KEY[11]) << 24);
        keyFour  = uint32_t(COMMAND_KEY[12]) | (uint32_t(COMMAND_KEY[13]) << 8) | (uint32_t(COMMAND_KEY[14]) << 16) | (uint32_t(COMMAND_KEY[15]) << 24);
    }

    uint32_t sum   = 0;
    uint32_t delta = 0x9E3779B9;

    for (int i = 0; i < 32; i++) {
        sum += delta;
        dataOne += (((dataTwo << 4) + keyOne) ^ (dataTwo + sum) ^ ((dataTwo >> 5) + keyTwo));
        dataTwo += (((dataOne << 4) + keyThree) ^ (dataOne + sum) ^ ((dataOne >> 5) + keyFour));
    }

    std::array<uint8_t, 8> encrypted = {uint8_t(dataOne & 0xFF), uint8_t((dataOne >> 8) & 0xFF),
                                        uint8_t((dataOne >> 16) & 0xFF), uint8_t((dataOne >> 24) & 0xFF),
                                        uint8_t(dataTwo & 0xFF), uint8_t((dataTwo >> 8) & 0xFF),
                                        uint8_t((dataTwo >> 16) & 0xFF), uint8_t((dataTwo >> 24) & 0xFF)};
    return encrypted;
}

std::array<uint8_t, 16> DimensionsToypad::GenerateFigureKey(const std::array<uint8_t, 0x2D * 0x04> &buf) {
    std::array<uint8_t, 7> uid = {buf[0], buf[1], buf[2], buf[4], buf[5], buf[6], buf[7]};

    uint32_t scrambleA = Scramble(uid, 3);
    uint32_t scrambleB = Scramble(uid, 4);
    uint32_t scrambleC = Scramble(uid, 5);
    uint32_t scrambleD = Scramble(uid, 6);

    return {uint8_t((scrambleA >> 24) & 0xFF), uint8_t((scrambleA >> 16) & 0xFF),
            uint8_t((scrambleA >> 8) & 0xFF), uint8_t(scrambleA & 0xFF),
            uint8_t((scrambleB >> 24) & 0xFF), uint8_t((scrambleB >> 16) & 0xFF),
            uint8_t((scrambleB >> 8) & 0xFF), uint8_t(scrambleB & 0xFF),
            uint8_t((scrambleC >> 24) & 0xFF), uint8_t((scrambleC >> 16) & 0xFF),
            uint8_t((scrambleC >> 8) & 0xFF), uint8_t(scrambleC & 0xFF),
            uint8_t((scrambleD >> 24) & 0xFF), uint8_t((scrambleD >> 16) & 0xFF),
            uint8_t((scrambleD >> 8) & 0xFF), uint8_t(scrambleD & 0xFF)};
}

uint32_t DimensionsToypad::Scramble(const std::array<uint8_t, 7> &uid, uint8_t count) {
    std::vector<uint8_t> toScramble;
    toScramble.reserve(uid.size() + CHAR_CONSTANT.size());
    for (uint8_t x : uid) {
        toScramble.push_back(x);
    }
    for (uint8_t c : CHAR_CONSTANT) {
        toScramble.push_back(c);
    }
    toScramble[(count * 4) - 1] = 0xaa;

    std::array<uint8_t, 4> randomized = DimensionsRandomize(toScramble, count);

    return (uint32_t(randomized[0]) << 24) | (uint32_t(randomized[1]) << 16) | (uint32_t(randomized[2]) << 8) | uint32_t(randomized[3]);
}

std::array<uint8_t, 4> DimensionsToypad::PWDGenerate(const std::array<uint8_t, 0x2D * 0x04> &buf) {
    std::array<uint8_t, 7> uid = {buf[0], buf[1], buf[2], buf[4], buf[5], buf[6], buf[7]};

    std::vector<uint8_t> pwdCalc = {PWD_CONSTANT.begin(), PWD_CONSTANT.end() - 1};
    for (uint8_t i = 0; i < uid.size(); i++) {
        pwdCalc.insert(pwdCalc.begin() + i, uid[i]);
    }

    return DimensionsRandomize(pwdCalc, 8);
}

std::array<uint8_t, 4> DimensionsToypad::DimensionsRandomize(const std::vector<uint8_t> key, uint8_t count) {
    uint32_t scrambled = 0;
    for (uint8_t i = 0; i < count; i++) {
        const uint32_t v4 = std::rotr(scrambled, 25);
        const uint32_t v5 = std::rotr(scrambled, 10);
        const uint32_t b  = uint32_t(key[i * 4]) | (uint32_t(key[(i * 4) + 1]) << 8) | (uint32_t(key[(i * 4) + 2]) << 16) | (uint32_t(key[(i * 4) + 3]) << 24);
        scrambled         = b + v4 + v5 - scrambled;
    }
    return {uint8_t(scrambled & 0xFF), uint8_t(scrambled >> 8 & 0xFF), uint8_t(scrambled >> 16 & 0xFF), uint8_t(scrambled >> 24 & 0xFF)};
}

uint32_t DimensionsToypad::GetFigureId(const std::array<uint8_t, 0x2D * 0x04> &buf) {
    const std::array<uint8_t, 16> figureKey = GenerateFigureKey(buf);

    const std::span<const uint8_t, 8> modelNumber = std::span<const uint8_t, 8>{buf.begin() + (36 * 4), 8};

    const std::array<uint8_t, 8> decrypted = Decrypt(modelNumber, figureKey);

    const uint32_t figNum = uint32_t(decrypted[0]) | (uint32_t(decrypted[1]) << 8) | (uint32_t(decrypted[2]) << 16) | (uint32_t(decrypted[3]) << 24);
    // Characters have their model number encrypted in page 36
    if (figNum < 1000) {
        return figNum;
    }
    // Vehicles/Gadgets have their model number written as little endian in page 36
    return uint32_t(modelNumber[0]) | (uint32_t(modelNumber[1]) << 8) | (uint32_t(modelNumber[2]) << 16) | (uint32_t(modelNumber[3]) << 24);
}

DimensionsToypad::DimensionsMini &
DimensionsToypad::GetFigureByIndex(uint8_t index) {
    return m_figures[index];
}

void DimensionsToypad::RandomUID(std::array<uint8_t, 0x2D * 0x04> &uid_buffer) {
    uid_buffer[0] = 0x04;
    uid_buffer[7] = 0x80;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    uid_buffer[1] = dist(mt);
    uid_buffer[2] = dist(mt);
    uid_buffer[4] = dist(mt);
    uid_buffer[5] = dist(mt);
    uid_buffer[6] = dist(mt);
}

uint8_t DimensionsToypad::GenerateChecksum(const std::array<uint8_t, 32> &data,
                                           int num_of_bytes) const {
    int checksum = 0;
    for (int i = 0; i < num_of_bytes; i++) {
        checksum += data[i];
    }
    return (checksum & 0xFF);
}

void DimensionsToypad::DimensionsMini::Save() {
    if (filePath.empty()) {
        DEBUG_FUNCTION_LINE("No Dimensions file present to save");
        return;
    }

    int result = FSUtils::WriteToFile(filePath.c_str(), data.data(), data.size());
    DEBUG_FUNCTION_LINE("WriteToFile returned %d", result);
}

std::map<const uint32_t, const char *> DimensionsToypad::GetListMinifigs() {
    return s_listMinis;
}

std::map<const uint32_t, const char *> DimensionsToypad::GetListTokens() {
    return s_listTokens;
}

std::string DimensionsToypad::FindFigure(uint32_t figNum) {
    for (const auto &it : GetListMinifigs()) {
        if (it.first == figNum) {
            return it.second;
        }
    }
    for (const auto &it : GetListTokens()) {
        if (it.first == figNum) {
            return it.second;
        }
    }
    return std::format("Unknown ({})", figNum);
}

std::array<std::optional<uint32_t>, 7> DimensionsToypad::GetCurrentFigures() {
    std::array<std::optional<uint32_t>, 7> currentFigures = {};
    for (uint8_t i = 0; i < 7; i++) {
        const DimensionsMini &figure = GetFigureByIndex(i);
        if (figure.index != 255) {
            currentFigures[i] = figure.id;
        } else {
            currentFigures[i] = std::nullopt;
        }
    }
    return currentFigures;
}

std::array<DimensionsToypad::DimensionsLEDColor, 3> DimensionsToypad::GetPadColors() {
    return {m_colorTop, m_colorLeft, m_colorRight};
}