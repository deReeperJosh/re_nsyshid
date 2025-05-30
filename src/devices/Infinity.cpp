#include "Infinity.h"

#include <format>

#include "utils/logger.h"

#include "utils/aes.hpp"
#include "utils/sha1.h"

static constexpr std::array<uint8_t, 32> SHA1_CONSTANT = {
        0xAF, 0x62, 0xD2, 0xEC, 0x04, 0x91, 0x96, 0x8C, 0xC5, 0x2A, 0x1A, 0x71, 0x65, 0xF8, 0x65, 0xFE,
        0x28, 0x63, 0x29, 0x20, 0x44, 0x69, 0x73, 0x6e, 0x65, 0x79, 0x20, 0x32, 0x30, 0x31, 0x33};

InfinityBase g_infinitybase;

const std::map<const uint32_t, const std::pair<const uint8_t, const char *>> s_listFigures = {
        {0x0F4241, {1, "Mr. Incredible"}},
        {0x0F4242, {1, "Sulley"}},
        {0x0F4243, {1, "Jack Sparrow"}},
        {0x0F4244, {1, "Lone Ranger"}},
        {0x0F4245, {1, "Tonto"}},
        {0x0F4246, {1, "Lightning McQueen"}},
        {0x0F4247, {1, "Holley Shiftwell"}},
        {0x0F4248, {1, "Buzz Lightyear"}},
        {0x0F4249, {1, "Jessie"}},
        {0x0F424A, {1, "Mike"}},
        {0x0F424B, {1, "Mrs. Incredible"}},
        {0x0F424C, {1, "Hector Barbossa"}},
        {0x0F424D, {1, "Davy Jones"}},
        {0x0F424E, {1, "Randy"}},
        {0x0F424F, {1, "Syndrome"}},
        {0x0F4250, {1, "Woody"}},
        {0x0F4251, {1, "Mater"}},
        {0x0F4252, {1, "Dash"}},
        {0x0F4253, {1, "Violet"}},
        {0x0F4254, {1, "Francesco Bernoulli"}},
        {0x0F4255, {1, "Sorcerer's Apprentice Mickey"}},
        {0x0F4256, {1, "Jack Skellington"}},
        {0x0F4257, {1, "Rapunzel"}},
        {0x0F4258, {1, "Anna"}},
        {0x0F4259, {1, "Elsa"}},
        {0x0F425A, {1, "Phineas"}},
        {0x0F425B, {1, "Agent P"}},
        {0x0F425C, {1, "Wreck-It Ralph"}},
        {0x0F425D, {1, "Vanellope"}},
        {0x0F425E, {1, "Mr. Incredible (Crystal)"}},
        {0x0F425F, {1, "Jack Sparrow (Crystal)"}},
        {0x0F4260, {1, "Sulley (Crystal)"}},
        {0x0F4261, {1, "Lightning McQueen (Crystal)"}},
        {0x0F4262, {1, "Lone Ranger (Crystal)"}},
        {0x0F4263, {1, "Buzz Lightyear (Crystal)"}},
        {0x0F4264, {1, "Agent P (Crystal)"}},
        {0x0F4265, {1, "Sorcerer's Apprentice Mickey (Crystal)"}},
        {0x0F4266, {1, "Buzz Lightyear (Glowing)"}},
        {0x0F42A4, {2, "Captain America"}},
        {0x0F42A5, {2, "Hulk"}},
        {0x0F42A6, {2, "Iron Man"}},
        {0x0F42A7, {2, "Thor"}},
        {0x0F42A8, {2, "Groot"}},
        {0x0F42A9, {2, "Rocket Raccoon"}},
        {0x0F42AA, {2, "Star-Lord"}},
        {0x0F42AB, {2, "Spider-Man"}},
        {0x0F42AC, {2, "Nick Fury"}},
        {0x0F42AD, {2, "Black Widow"}},
        {0x0F42AE, {2, "Hawkeye"}},
        {0x0F42AF, {2, "Drax"}},
        {0x0F42B0, {2, "Gamora"}},
        {0x0F42B1, {2, "Iron Fist"}},
        {0x0F42B2, {2, "Nova"}},
        {0x0F42B3, {2, "Venom"}},
        {0x0F42B4, {2, "Donald Duck"}},
        {0x0F42B5, {2, "Aladdin"}},
        {0x0F42B6, {2, "Stitch"}},
        {0x0F42B7, {2, "Merida"}},
        {0x0F42B8, {2, "Tinker Bell"}},
        {0x0F42B9, {2, "Maleficent"}},
        {0x0F42BA, {2, "Hiro"}},
        {0x0F42BB, {2, "Baymax"}},
        {0x0F42BC, {2, "Loki"}},
        {0x0F42BD, {2, "Ronan"}},
        {0x0F42BE, {2, "Green Goblin"}},
        {0x0F42BF, {2, "Falcon"}},
        {0x0F42C0, {2, "Yondu"}},
        {0x0F42C1, {2, "Jasmine"}},
        {0x0F42C6, {2, "Black Suit Spider-Man"}},
        {0x0F42D6, {3, "Sam Flynn"}},
        {0x0F42D7, {3, "Quorra"}},
        {0x0F4308, {3, "Anakin Skywalker"}},
        {0x0F4309, {3, "Obi-Wan Kenobi"}},
        {0x0F430A, {3, "Yoda"}},
        {0x0F430B, {3, "Ahsoka Tano"}},
        {0x0F430C, {3, "Darth Maul"}},
        {0x0F430E, {3, "Luke Skywalker"}},
        {0x0F430F, {3, "Han Solo"}},
        {0x0F4310, {3, "Princess Leia"}},
        {0x0F4311, {3, "Chewbacca"}},
        {0x0F4312, {3, "Darth Vader"}},
        {0x0F4313, {3, "Boba Fett"}},
        {0x0F4314, {3, "Ezra Bridger"}},
        {0x0F4315, {3, "Kanan Jarrus"}},
        {0x0F4316, {3, "Sabine Wren"}},
        {0x0F4317, {3, "Zeb Orrelios"}},
        {0x0F4318, {3, "Joy"}},
        {0x0F4319, {3, "Anger"}},
        {0x0F431A, {3, "Fear"}},
        {0x0F431B, {3, "Sadness"}},
        {0x0F431C, {3, "Disgust"}},
        {0x0F431D, {3, "Mickey Mouse"}},
        {0x0F431E, {3, "Minnie Mouse"}},
        {0x0F431F, {3, "Mulan"}},
        {0x0F4320, {3, "Olaf"}},
        {0x0F4321, {3, "Vision"}},
        {0x0F4322, {3, "Ultron"}},
        {0x0F4323, {3, "Ant-Man"}},
        {0x0F4325, {3, "Captain America - The First Avenger"}},
        {0x0F4326, {3, "Finn"}},
        {0x0F4327, {3, "Kylo Ren"}},
        {0x0F4328, {3, "Poe Dameron"}},
        {0x0F4329, {3, "Rey"}},
        {0x0F432B, {3, "Spot"}},
        {0x0F432C, {3, "Nick Wilde"}},
        {0x0F432D, {3, "Judy Hopps"}},
        {0x0F432E, {3, "Hulkbuster"}},
        {0x0F432F, {3, "Anakin Skywalker (Light FX)"}},
        {0x0F4330, {3, "Obi-Wan Kenobi (Light FX)"}},
        {0x0F4331, {3, "Yoda (Light FX)"}},
        {0x0F4332, {3, "Luke Skywalker (Light FX)"}},
        {0x0F4333, {3, "Darth Vader (Light FX)"}},
        {0x0F4334, {3, "Kanan Jarrus (Light FX)"}},
        {0x0F4335, {3, "Kylo Ren (Light FX)"}},
        {0x0F4336, {3, "Black Panther"}},
        {0x0F436C, {3, "Nemo"}},
        {0x0F436D, {3, "Dory"}},
        {0x0F436E, {3, "Baloo"}},
        {0x0F436F, {3, "Alice"}},
        {0x0F4370, {3, "Mad Hatter"}},
        {0x0F4371, {3, "Time"}},
        {0x0F4372, {3, "Peter Pan"}},
        {0x1E8481, {1, "Starter Play Set"}},
        {0x1E8482, {1, "Lone Ranger Play Set"}},
        {0x1E8483, {1, "Cars Play Set"}},
        {0x1E8484, {1, "Toy Story in Space Play Set"}},
        {0x1E84E4, {2, "Marvel's The Avengers Play Set"}},
        {0x1E84E5, {2, "Marvel's Spider-Man Play Set"}},
        {0x1E84E6, {2, "Marvel's Guardians of the Galaxy Play Set"}},
        {0x1E84E7, {2, "Assault on Asgard"}},
        {0x1E84E8, {2, "Escape from the Kyln"}},
        {0x1E84E9, {2, "Stitch's Tropical Rescue"}},
        {0x1E84EA, {2, "Brave Forest Siege"}},
        {0x1E8548, {3, "Inside Out Play Set"}},
        {0x1E854A, {3, "Star Wars: Twilight of the Republic Play Set"}},
        {0x1E854B, {3, "Star Wars: Rise Against the Empire Play Set"}},
        {0x1E854C, {3, "Star Wars: The Force Awakens Play Set"}},
        {0x1E854D, {3, "Marvel Battlegrounds Play Set"}},
        {0x1E854E, {3, "Toy Box Speedway"}},
        {0x1E854F, {3, "Toy Box Takeover"}},
        {0x1E85AC, {3, "Finding Dory Play Set"}},
        {0x2DC6C3, {1, "Bolt's Super Strength"}},
        {0x2DC6C4, {1, "Ralph's Power of Destruction"}},
        {0x2DC6C5, {1, "Chernabog's Power"}},
        {0x2DC6C6, {1, "C.H.R.O.M.E. Damage Increaser"}},
        {0x2DC6C7, {1, "Dr. Doofenshmirtz's Damage-Inator!"}},
        {0x2DC6C8, {1, "Electro-Charge"}},
        {0x2DC6C9, {1, "Fix-It Felix's Repair Power"}},
        {0x2DC6CA, {1, "Rapunzel's Healing"}},
        {0x2DC6CB, {1, "C.H.R.O.M.E. Armor Shield"}},
        {0x2DC6CC, {1, "Star Command Shield"}},
        {0x2DC6CD, {1, "Violet's Force Field"}},
        {0x2DC6CE, {1, "Pieces of Eight"}},
        {0x2DC6CF, {1, "Scrooge McDuck's Lucky Dime"}},
        {0x2DC6D0, {1, "User Control"}},
        {0x2DC6D1, {1, "Sorcerer Mickey's Hat"}},
        {0x2DC6FE, {1, "Emperor Zurg's Wrath"}},
        {0x2DC6FF, {1, "Merlin's Summon"}},
        {0x2DC765, {2, "Enchanted Rose"}},
        {0x2DC766, {2, "Mulan's Training Uniform"}},
        {0x2DC767, {2, "Flubber"}},
        {0x2DC768, {2, "S.H.I.E.L.D. Helicarrier Strike"}},
        {0x2DC769, {2, "Zeus' Thunderbolts"}},
        {0x2DC76A, {2, "King Louie's Monkeys"}},
        {0x2DC76B, {2, "Infinity Gauntlet"}},
        {0x2DC76D, {2, "Sorcerer Supreme"}},
        {0x2DC76E, {2, "Maleficent's Spell Cast"}},
        {0x2DC76F, {2, "Chernabog's Spirit Cyclone"}},
        {0x2DC770, {2, "Marvel Team-Up: Capt. Marvel"}},
        {0x2DC771, {2, "Marvel Team-Up: Iron Patriot"}},
        {0x2DC772, {2, "Marvel Team-Up: Ant-Man"}},
        {0x2DC773, {2, "Marvel Team-Up: White Tiger"}},
        {0x2DC774, {2, "Marvel Team-Up: Yondu"}},
        {0x2DC775, {2, "Marvel Team-Up: Winter Soldier"}},
        {0x2DC776, {2, "Stark Arc Reactor"}},
        {0x2DC777, {2, "Gamma Rays"}},
        {0x2DC778, {2, "Alien Symbiote"}},
        {0x2DC779, {2, "All for One"}},
        {0x2DC77A, {2, "Sandy Claws Surprise"}},
        {0x2DC77B, {2, "Glory Days"}},
        {0x2DC77C, {2, "Cursed Pirate Gold"}},
        {0x2DC77D, {2, "Sentinel of Liberty"}},
        {0x2DC77E, {2, "The Immortal Iron Fist"}},
        {0x2DC77F, {2, "Space Armor"}},
        {0x2DC780, {2, "Rags to Riches"}},
        {0x2DC781, {2, "Ultimate Falcon"}},
        {0x2DC788, {3, "Tomorrowland Time Bomb"}},
        {0x2DC78E, {3, "Galactic Team-Up: Mace Windu"}},
        {0x2DC791, {3, "Luke's Rebel Alliance Flight Suit Costume"}},
        {0x2DC798, {3, "Finn's Stormtrooper Costume"}},
        {0x2DC799, {3, "Poe's Resistance Jacket"}},
        {0x2DC79A, {3, "Resistance Tactical Strike"}},
        {0x2DC79E, {3, "Officer Nick Wilde"}},
        {0x2DC79F, {3, "Meter Maid Judy"}},
        {0x2DC7A2, {3, "Darkhawk's Blast"}},
        {0x2DC7A3, {3, "Cosmic Cube Blast"}},
        {0x2DC7A4, {3, "Princess Leia's Boushh Disguise"}},
        {0x2DC7A6, {3, "Nova Corps Strike"}},
        {0x2DC7A7, {3, "King Mickey"}},
        {0x3D0912, {1, "Mickey's Car"}},
        {0x3D0913, {1, "Cinderella's Coach"}},
        {0x3D0914, {1, "Electric Mayhem Bus"}},
        {0x3D0915, {1, "Cruella De Vil's Car"}},
        {0x3D0916, {1, "Pizza Planet Delivery Truck"}},
        {0x3D0917, {1, "Mike's New Car"}},
        {0x3D0919, {1, "Parking Lot Tram"}},
        {0x3D091A, {1, "Captain Hook's Ship"}},
        {0x3D091B, {1, "Dumbo"}},
        {0x3D091C, {1, "Calico Helicopter"}},
        {0x3D091D, {1, "Maximus"}},
        {0x3D091E, {1, "Angus"}},
        {0x3D091F, {1, "Abu the Elephant"}},
        {0x3D0920, {1, "Headless Horseman's Horse"}},
        {0x3D0921, {1, "Phillipe"}},
        {0x3D0922, {1, "Khan"}},
        {0x3D0923, {1, "Tantor"}},
        {0x3D0924, {1, "Dragon Firework Cannon"}},
        {0x3D0925, {1, "Stitch's Blaster"}},
        {0x3D0926, {1, "Toy Story Mania Blaster"}},
        {0x3D0927, {1, "Flamingo Croquet Mallet"}},
        {0x3D0928, {1, "Carl Fredricksen's Cane"}},
        {0x3D0929, {1, "Hangin' Ten Stitch With Surfboard"}},
        {0x3D092A, {1, "Condorman Glider"}},
        {0x3D092B, {1, "WALL-E's Fire Extinguisher"}},
        {0x3D092C, {1, "On the Grid"}},
        {0x3D092D, {1, "WALL-E's Collection"}},
        {0x3D092E, {1, "King Candy's Dessert Toppings"}},
        {0x3D0930, {1, "Victor's Experiments"}},
        {0x3D0931, {1, "Jack's Scary Decorations"}},
        {0x3D0933, {1, "Frozen Flourish"}},
        {0x3D0934, {1, "Rapunzel's Kingdom"}},
        {0x3D0935, {1, "TRON Interface"}},
        {0x3D0936, {1, "Buy N Large Atmosphere"}},
        {0x3D0937, {1, "Sugar Rush Sky"}},
        {0x3D0939, {1, "New Holland Skyline"}},
        {0x3D093A, {1, "Halloween Town Sky"}},
        {0x3D093C, {1, "Chill in the Air"}},
        {0x3D093D, {1, "Rapunzel's Birthday Sky"}},
        {0x3D0940, {1, "Astro Blasters Space Cruiser"}},
        {0x3D0941, {1, "Marlin's Reef"}},
        {0x3D0942, {1, "Nemo's Seascape"}},
        {0x3D0943, {1, "Alice's Wonderland"}},
        {0x3D0944, {1, "Tulgey Wood"}},
        {0x3D0945, {1, "Tri-State Area Terrain"}},
        {0x3D0946, {1, "Danville Sky"}},
        {0x3D0965, {2, "Stark Tech"}},
        {0x3D0966, {2, "Spider-Streets"}},
        {0x3D0967, {2, "World War Hulk"}},
        {0x3D0968, {2, "Gravity Falls Forest"}},
        {0x3D0969, {2, "Neverland"}},
        {0x3D096A, {2, "Simba's Pridelands"}},
        {0x3D096C, {2, "Calhoun's Command"}},
        {0x3D096D, {2, "Star-Lord's Galaxy"}},
        {0x3D096E, {2, "Dinosaur World"}},
        {0x3D096F, {2, "Groot's Roots"}},
        {0x3D0970, {2, "Mulan's Countryside"}},
        {0x3D0971, {2, "The Sands of Agrabah"}},
        {0x3D0974, {2, "A Small World"}},
        {0x3D0975, {2, "View from the Suit"}},
        {0x3D0976, {2, "Spider-Sky"}},
        {0x3D0977, {2, "World War Hulk Sky"}},
        {0x3D0978, {2, "Gravity Falls Sky"}},
        {0x3D0979, {2, "Second Star to the Right"}},
        {0x3D097A, {2, "The King's Domain"}},
        {0x3D097C, {2, "CyBug Swarm"}},
        {0x3D097D, {2, "The Rip"}},
        {0x3D097E, {2, "Forgotten Skies"}},
        {0x3D097F, {2, "Groot's View"}},
        {0x3D0980, {2, "The Middle Kingdom"}},
        {0x3D0984, {2, "Skies of the World"}},
        {0x3D0985, {2, "S.H.I.E.L.D. Containment Truck"}},
        {0x3D0986, {2, "Main Street Electrical Parade Float"}},
        {0x3D0987, {2, "Mr. Toad's Motorcar"}},
        {0x3D0988, {2, "Le Maximum"}},
        {0x3D0989, {2, "Alice in Wonderland's Caterpillar"}},
        {0x3D098A, {2, "Eglantine's Motorcycle"}},
        {0x3D098B, {2, "Medusa's Swamp Mobile"}},
        {0x3D098C, {2, "Hydra Motorcycle"}},
        {0x3D098D, {2, "Darkwing Duck's Ratcatcher"}},
        {0x3D098F, {2, "The USS Swinetrek"}},
        {0x3D0991, {2, "Spider-Copter"}},
        {0x3D0992, {2, "Aerial Area Rug"}},
        {0x3D0993, {2, "Jack-O-Lantern's Glider"}},
        {0x3D0994, {2, "Spider-Buggy"}},
        {0x3D0995, {2, "Jack Skellington's Reindeer"}},
        {0x3D0996, {2, "Fantasyland Carousel Horse"}},
        {0x3D0997, {2, "Odin's Horse"}},
        {0x3D0998, {2, "Gus the Mule"}},
        {0x3D099A, {2, "Darkwing Duck's Grappling Gun"}},
        {0x3D099C, {2, "Ghost Rider's Chain Whip"}},
        {0x3D099D, {2, "Lew Zealand's Boomerang Fish"}},
        {0x3D099E, {2, "Sergeant Calhoun's Blaster"}},
        {0x3D09A0, {2, "Falcon's Wings"}},
        {0x3D09A1, {2, "Mabel's Kittens for Fists"}},
        {0x3D09A2, {2, "Jim Hawkins' Solar Board"}},
        {0x3D09A3, {2, "Black Panther's Vibranium Knives"}},
        {0x3D09A4, {2, "Cloak of Levitation"}},
        {0x3D09A5, {2, "Aladdin's Magic Carpet"}},
        {0x3D09A6, {2, "Honey Lemon's Ice Capsules"}},
        {0x3D09A7, {2, "Jasmine's Palace View"}},
        {0x3D09C1, {2, "Lola"}},
        {0x3D09C2, {2, "Spider-Cycle"}},
        {0x3D09C3, {2, "The Avenjet"}},
        {0x3D09C4, {2, "Spider-Glider"}},
        {0x3D09C5, {2, "Light Cycle"}},
        {0x3D09C6, {2, "Light Jet"}},
        {0x3D09C9, {3, "Retro Ray Gun"}},
        {0x3D09CA, {3, "Tomorrowland Futurescape"}},
        {0x3D09CB, {3, "Tomorrowland Stratosphere"}},
        {0x3D09CC, {3, "Skies Over Felucia"}},
        {0x3D09CD, {3, "Forests of Felucia"}},
        {0x3D09CF, {3, "General Grievous' Wheel Bike"}},
        {0x3D09D2, {3, "Slave I Flyer"}},
        {0x3D09D3, {3, "Y-Wing Fighter"}},
        {0x3D09D4, {3, "Arlo"}},
        {0x3D09D5, {3, "Nash"}},
        {0x3D09D6, {3, "Butch"}},
        {0x3D09D7, {3, "Ramsey"}},
        {0x3D09DC, {3, "Stars Over Sahara Square"}},
        {0x3D09DD, {3, "Sahara Square Sands"}},
        {0x3D09E0, {3, "Ghost Rider's Motorcycle"}},
        {0x3D09E5, {3, "Quad Jumper"}}};

InfinityUSBDevice::InfinityUSBDevice()
    : Device(0x6F0E, 0x2901, 0, 0, 0, 32, 32) {
}

InfinityUSBDevice::~InfinityUSBDevice() = default;

bool InfinityUSBDevice::GetDescriptor(uint8_t descType,
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

bool InfinityUSBDevice::SetDescriptor(uint8_t descType,
                                      uint8_t descIndex,
                                      uint16_t lang,
                                      uint8_t *buffer,
                                      uint32_t bufferLength) {
    return true;
}

bool InfinityUSBDevice::GetReport(uint8_t *buffer,
                                  uint32_t bufferLength) {
    return true;
}

bool InfinityUSBDevice::SetReport(uint8_t *buffer,
                                  uint32_t bufferLength) {
    return true;
}

bool InfinityUSBDevice::GetIdle(uint8_t ifIndex,
                                uint8_t reportId,
                                uint8_t *duration) {
    return true;
}

bool InfinityUSBDevice::SetIdle(uint8_t ifIndex,
                                uint8_t reportId,
                                uint8_t duration) {
    return true;
}

bool InfinityUSBDevice::GetProtocol(uint8_t ifIndex,
                                    uint8_t *protocol) {
    return true;
}

bool InfinityUSBDevice::SetProtocol(uint8_t ifIndex,
                                    uint8_t protocol) {
    return true;
}

bool InfinityUSBDevice::Read(uint8_t *buffer,
                             uint32_t bufferLength) {
    memcpy(buffer, g_infinitybase.GetStatus().data(), bufferLength);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

bool InfinityUSBDevice::Write(uint8_t *buffer,
                              uint32_t bufferLength) {
    g_infinitybase.SendCommand(buffer, bufferLength);
    return true;
}

void InfinityBase::SendCommand(uint8_t *buf, uint32_t length) {
    const uint8_t command  = buf[2];
    const uint8_t sequence = buf[3];

    std::array<uint8_t, 32> q_result{};

    switch (command) {
        case 0x80: {
            q_result = {0xaa, 0x15, 0x00, 0x00, 0x0f, 0x01, 0x00, 0x03,
                        0x02, 0x09, 0x09, 0x43, 0x20, 0x32, 0x62, 0x36,
                        0x36, 0x4b, 0x34, 0x99, 0x67, 0x31, 0x93, 0x8c};
            break;
        }
        case 0x81: {
            // Initiate Challenge
            g_infinitybase.DescrambleAndSeed(buf, sequence, q_result);
            break;
        }
        case 0x83: {
            // Challenge Response
            g_infinitybase.GetNextAndScramble(sequence, q_result);
            break;
        }
        case 0x90:
        case 0x92:
        case 0x93:
        case 0x95:
        case 0x96: {
            // Color commands
            g_infinitybase.GetBlankResponse(sequence, q_result);
            break;
        }
        case 0xA1: {
            // Get Present Figures
            g_infinitybase.GetPresentFigures(sequence, q_result);
            break;
        }
        case 0xA2: {
            // Read Block from Figure
            g_infinitybase.QueryBlock(buf[4], buf[5], q_result, sequence);
            break;
        }
        case 0xA3: {
            // Write block to figure
            g_infinitybase.WriteBlock(buf[4], buf[5], &buf[7], q_result, sequence);
            break;
        }
        case 0xB4: {
            // Get figure ID
            g_infinitybase.GetFigureIdentifier(buf[4], sequence, q_result);
            break;
        }
        case 0xB5: {
            // Get status?
            g_infinitybase.GetBlankResponse(sequence, q_result);
            break;
        }
        default:
            DEBUG_FUNCTION_LINE_ERR("Unknown Infinity Base Command: %02X", command);
            break;
    }

    m_queries.push(q_result);
}


std::array<uint8_t, 32> InfinityBase::GetStatus() {
    std::array<uint8_t, 32> response = {};

    bool responded = false;

    do {
        if (!m_figureAddedRemovedResponses.empty()) {
            memcpy(response.data(), m_figureAddedRemovedResponses.front().data(),
                   0x20);
            m_figureAddedRemovedResponses.pop();
            responded = true;
        } else if (!m_queries.empty()) {
            memcpy(response.data(), m_queries.front().data(), 0x20);
            m_queries.pop();
            responded = true;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        /* code */
    } while (!responded);

    return response;
}

void InfinityBase::GetBlankResponse(uint8_t sequence,
                                    std::array<uint8_t, 32> &replyBuf) {
    replyBuf[0] = 0xaa;
    replyBuf[1] = 0x01;
    replyBuf[2] = sequence;
    replyBuf[3] = GenerateChecksum(replyBuf, 3);
}

void InfinityBase::DescrambleAndSeed(uint8_t *buf, uint8_t sequence,
                                     std::array<uint8_t, 32> &replyBuf) {
    uint64_t value = uint64_t(buf[4]) << 56 | uint64_t(buf[5]) << 48 |
                     uint64_t(buf[6]) << 40 | uint64_t(buf[7]) << 32 |
                     uint64_t(buf[8]) << 24 | uint64_t(buf[9]) << 16 |
                     uint64_t(buf[10]) << 8 | uint64_t(buf[11]);
    uint32_t seed = Descramble(value);
    GenerateSeed(seed);
    GetBlankResponse(sequence, replyBuf);
}


void InfinityBase::GetNextAndScramble(uint8_t sequence,
                                      std::array<uint8_t, 32> &replyBuf) {
    const uint32_t nextRandom          = GetNext();
    const uint64_t scrambledNextRandom = Scramble(nextRandom, 0);
    replyBuf                           = {0xAA, 0x09, sequence};
    replyBuf[3]                        = uint8_t((scrambledNextRandom >> 56) & 0xFF);
    replyBuf[4]                        = uint8_t((scrambledNextRandom >> 48) & 0xFF);
    replyBuf[5]                        = uint8_t((scrambledNextRandom >> 40) & 0xFF);
    replyBuf[6]                        = uint8_t((scrambledNextRandom >> 32) & 0xFF);
    replyBuf[7]                        = uint8_t((scrambledNextRandom >> 24) & 0xFF);
    replyBuf[8]                        = uint8_t((scrambledNextRandom >> 16) & 0xFF);
    replyBuf[9]                        = uint8_t((scrambledNextRandom >> 8) & 0xFF);
    replyBuf[10]                       = uint8_t(scrambledNextRandom & 0xFF);
    replyBuf[11]                       = GenerateChecksum(replyBuf, 11);
}

void InfinityBase::GetPresentFigures(uint8_t sequence,
                                     std::array<uint8_t, 32> &replyBuf) {
    int x = 3;
    for (uint8_t i = 0; i < m_figures.size(); i++) {
        uint8_t slot = (i < 3) ? 0x10 : (i < 6) ? 0x20
                                               : 0x30;
        if (m_figures[i].present) {
            replyBuf[x]     = slot + m_figures[i].orderAdded;
            replyBuf[x + 1] = 0x09;
            x += 2;
        }
    }
    replyBuf[0] = 0xaa;
    replyBuf[1] = x - 2;
    replyBuf[2] = sequence;
    replyBuf[x] = GenerateChecksum(replyBuf, x);
}

void InfinityBase::QueryBlock(uint8_t fig_num, uint8_t block,
                              std::array<uint8_t, 32> &replyBuf,
                              uint8_t sequence) {
    std::lock_guard lock(m_infinityMutex);

    InfinityFigure &figure = GetFigureByOrder(fig_num);

    replyBuf[0]              = 0xaa;
    replyBuf[1]              = 0x12;
    replyBuf[2]              = sequence;
    replyBuf[3]              = 0x00;
    const uint8_t file_block = (block == 0) ? 1 : (block * 4);
    if (figure.present && file_block < 20) {
        memcpy(&replyBuf[4], figure.data.data() + (16 * file_block), 16);
    }
    replyBuf[20] = GenerateChecksum(replyBuf, 20);
}

void InfinityBase::WriteBlock(uint8_t fig_num, uint8_t block,
                              const uint8_t *to_write_buf,
                              std::array<uint8_t, 32> &replyBuf,
                              uint8_t sequence) {
    std::lock_guard lock(m_infinityMutex);

    InfinityFigure &figure = GetFigureByOrder(fig_num);

    if (figure.orderAdded == 255 || !figure.present) {
        DEBUG_FUNCTION_LINE_INFO("Couldn't find figure with order %u", fig_num);
    }

    replyBuf[0]              = 0xaa;
    replyBuf[1]              = 0x02;
    replyBuf[2]              = sequence;
    replyBuf[3]              = 0x00;
    const uint8_t file_block = (block == 0) ? 1 : (block * 4);
    if (figure.present && file_block < 20) {
        memcpy(figure.data.data() + (file_block * 16), to_write_buf, 16);
        figure.Save();
    }
    replyBuf[4] = GenerateChecksum(replyBuf, 4);
}

void InfinityBase::GetFigureIdentifier(uint8_t fig_num, uint8_t sequence,
                                       std::array<uint8_t, 32> &replyBuf) {
    std::lock_guard lock(m_infinityMutex);

    InfinityFigure &figure = GetFigureByOrder(fig_num);

    replyBuf[0] = 0xaa;
    replyBuf[1] = 0x09;
    replyBuf[2] = sequence;
    replyBuf[3] = 0x00;

    if (figure.present) {
        memcpy(&replyBuf[4], figure.data.data(), 7);
    }
    replyBuf[11] = GenerateChecksum(replyBuf, 11);
}

bool InfinityBase::RemoveFigure(uint8_t position) {
    std::lock_guard lock(m_infinityMutex);
    InfinityFigure &figure = m_figures[position];

    if (figure.present) {
        figure.figNum = 0;
        figure.Save();
        fclose(figure.infFile);
        figure.present = false;

        position = DeriveFigurePosition(position);
        if (position == 0) {
            return false;
        }

        std::array<uint8_t, 32> figureChangeResponse = {0xab, 0x04, position, 0x09, figure.orderAdded,
                                                        0x01};
        figureChangeResponse[6]                      = GenerateChecksum(figureChangeResponse, 6);
        m_figureAddedRemovedResponses.push(figureChangeResponse);

        return true;
    }
    return false;
}

uint32_t
InfinityBase::LoadFigure(const std::array<uint8_t, INF_FIGURE_SIZE> &buf,
                         FILE *inFile, uint8_t position) {
    std::lock_guard lock(m_infinityMutex);
    uint8_t orderAdded;

    std::vector<uint8_t> sha1Calc = {SHA1_CONSTANT.begin(), SHA1_CONSTANT.end() - 1};
    for (int i = 0; i < 7; i++) {
        sha1Calc.push_back(buf[i]);
    }

    std::array<uint8_t, 16> key = GenerateInfinityFigureKey(sha1Calc);

    std::array<uint8_t, 16> infinity_decrypted_block = {};
    std::array<uint8_t, 16> encryptedBlock           = {};
    memcpy(encryptedBlock.data(), &buf[16], 16);

    AES_ctx context;
    AES_init_ctx(&context, key.data());

    AES_ECB_decrypt(&context, encryptedBlock.data());
    memcpy(infinity_decrypted_block.data(), encryptedBlock.data(), 16);

    uint32_t number = uint32_t(infinity_decrypted_block[1]) << 16 | uint32_t(infinity_decrypted_block[2]) << 8 |
                      uint32_t(infinity_decrypted_block[3]);

    if ((position == 0 &&
         ((number > 0x1E8480 && number < 0x2DC6BF) || (number > 0x3D0900 && number < 0x4C4B3F))) ||
        ((position == 1 || position == 2) && (number > 0x3D0900 && number < 0x4C4B3F)) ||
        ((position == 3 || position == 6) && number < 0x1E847F) ||
        ((position == 4 || position == 5 || position == 7 || position == 8) &&
         (number > 0x2DC6C0 && number < 0x3D08FF))) {

        InfinityFigure &figure = m_figures[position];

        figure.infFile = std::move(inFile);
        figure.figNum  = number;
        memcpy(figure.data.data(), buf.data(), figure.data.size());
        figure.present = true;
        if (figure.orderAdded == 255) {
            figure.orderAdded = m_figureOrder;
            m_figureOrder++;
        }
        orderAdded = figure.orderAdded;

        position = DeriveFigurePosition(position);
        if (position == 0) {
            fclose(inFile);
            return 0;
        }

        std::array<uint8_t, 32> figureChangeResponse = {0xab, 0x04, position, 0x09, orderAdded, 0x00};
        figureChangeResponse[6]                      = GenerateChecksum(figureChangeResponse, 6);
        m_figureAddedRemovedResponses.push(figureChangeResponse);

        return number;
    }
    fclose(inFile);
    return 0;
}

std::pair<uint8_t, std::string> InfinityBase::FindFigure(uint32_t figNum) {
    for (const auto &it : GetFigureList()) {
        if (it.first == figNum) {
            return it.second;
        }
    }
    return {0, std::format("Unknown Figure ({})", figNum)};
}

std::map<const uint32_t, const std::pair<const uint8_t, const char *>> InfinityBase::GetFigureList() {
    return s_listFigures;
}

uint32_t InfinityBase::FindFigureFromSlot(uint8_t slot) {
    return m_figures[slot].figNum;
}

uint8_t InfinityBase::GenerateChecksum(const std::array<uint8_t, 32> &data,
                                       int numOfBytes) const {
    int checksum = 0;
    for (int i = 0; i < numOfBytes; i++) {
        checksum += data[i];
    }
    return (checksum & 0xFF);
}

uint32_t InfinityBase::Descramble(uint64_t numToDescramble) {
    uint64_t mask = 0x8E55AA1B3999E8AA;
    uint32_t ret  = 0;

    for (int i = 0; i < 64; i++) {
        if (mask & 0x8000000000000000) {
            ret = (ret << 1) | (numToDescramble & 0x01);
        }

        numToDescramble >>= 1;
        mask <<= 1;
    }

    return ret;
}

uint64_t InfinityBase::Scramble(uint32_t numToScramble, uint32_t garbage) {
    uint64_t mask = 0x8E55AA1B3999E8AA;
    uint64_t ret  = 0;

    for (int i = 0; i < 64; i++) {
        ret <<= 1;

        if ((mask & 1) != 0) {
            ret |= (numToScramble & 1);
            numToScramble >>= 1;
        } else {
            ret |= (garbage & 1);
            garbage >>= 1;
        }

        mask >>= 1;
    }

    return ret;
}

void InfinityBase::GenerateSeed(uint32_t seed) {
    m_randomA = 0xF1EA5EED;
    m_randomB = seed;
    m_randomC = seed;
    m_randomD = seed;

    for (int i = 0; i < 23; i++) {
        GetNext();
    }
}

uint32_t InfinityBase::GetNext() {
    uint32_t a   = m_randomA;
    uint32_t b   = m_randomB;
    uint32_t c   = m_randomC;
    uint32_t ret = std::rotl(m_randomB, 27);

    const uint32_t temp = (a + ((ret ^ 0xFFFFFFFF) + 1));
    b ^= std::rotl(c, 17);
    a = m_randomD;
    c += a;
    ret = b + temp;
    a += temp;

    m_randomC = a;
    m_randomA = b;
    m_randomB = c;
    m_randomD = ret;

    return ret;
}

InfinityBase::InfinityFigure &InfinityBase::GetFigureByOrder(uint8_t orderAdded) {
    for (uint8_t i = 0; i < m_figures.size(); i++) {
        if (m_figures[i].orderAdded == orderAdded) {
            return m_figures[i];
        }
    }
    InfinityBase::InfinityFigure figure = {};
    return figure;
}

uint8_t InfinityBase::DeriveFigurePosition(uint8_t position) {
    // In the added/removed response, position needs to be 1 for the hexagon, 2 for Player 1 and
    // Player 1's abilities, and 3 for Player 2 and Player 2's abilities. In the UI, positions 0, 1
    // and 2 represent the hexagon slot, 3, 4 and 5 represent Player 1's slot and 6, 7 and 8 represent
    // Player 2's slot.

    switch (position) {
        case 0:
        case 1:
        case 2:
            return 1;
        case 3:
        case 4:
        case 5:
            return 2;
        case 6:
        case 7:
        case 8:
            return 3;

        default:
            return 0;
    }
}

std::array<uint8_t, 16> InfinityBase::GenerateInfinityFigureKey(const std::vector<uint8_t> &sha1Data) {
    uint8_t digest[20];
    SHA1_CTX context;
    SHA1Init(&context);
    SHA1Update(&context, sha1Data.data(), sha1Data.size());
    SHA1Final(digest, &context);
    // Infinity AES keys are the first 16 bytes of the SHA1 Digest, every set of 4 bytes need to be
    // reversed due to endianness
    std::array<uint8_t, 16> key = {};
    for (int i = 0; i < 4; i++) {
        for (int x = 3; x >= 0; x--) {
            key[(3 - x) + (i * 4)] = digest[x + (i * 4)];
        }
    }
    return key;
}


void InfinityBase::InfinityFigure::Save() {
    if (!infFile)
        return;

    fseeko(infFile, 0, SEEK_SET);
    fwrite(data.data(), sizeof(data[0]), data.size(), infFile);
}
