#include "Skylander.h"
#include "utils/logger.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <format>
#include <optional>
#include <stdio.h>
#include <thread>
#include <wut.h>

SkylanderPortal g_skyportal;

const std::map<const std::pair<const uint16_t, const uint16_t>, const char *>
        s_listSkylanders = {
                {{0, 0x0000}, "Whirlwind"},
                {{0, 0x1801}, "Series 2 Whirlwind"},
                {{0, 0x1C02}, "Polar Whirlwind"},
                {{0, 0x2805}, "Horn Blast Whirlwind"},
                {{0, 0x3810}, "Eon's Elite Whirlwind"},
                {{1, 0x0000}, "Sonic Boom"},
                {{1, 0x1801}, "Series 2 Sonic Boom"},
                {{2, 0x0000}, "Warnado"},
                {{2, 0x2206}, "LightCore Warnado"},
                {{3, 0x0000}, "Lightning Rod"},
                {{3, 0x1801}, "Series 2 Lightning Rod"},
                {{4, 0x0000}, "Bash"},
                {{4, 0x1801}, "Series 2 Bash"},
                {{5, 0x0000}, "Terrafin"},
                {{5, 0x1801}, "Series 2 Terrafin"},
                {{5, 0x2805}, "Knockout Terrafin"},
                {{5, 0x3810}, "Eon's Elite Terrafin"},
                {{6, 0x0000}, "Dino Rang"},
                {{6, 0x4810}, "Eon's Elite Dino Rang"},
                {{7, 0x0000}, "Prism Break"},
                {{7, 0x1801}, "Series 2 Prism Break"},
                {{7, 0x2805}, "Hyper Beam Prism Break"},
                {{7, 0x1206}, "LightCore Prism Break"},
                {{8, 0x0000}, "Sunburn"},
                {{9, 0x0000}, "Eruptor"},
                {{9, 0x1801}, "Series 2 Eruptor"},
                {{9, 0x2C02}, "Volcanic Eruptor"},
                {{9, 0x2805}, "Lava Barf Eruptor"},
                {{9, 0x1206}, "LightCore Eruptor"},
                {{9, 0x3810}, "Eon's Elite Eruptor"},
                {{10, 0x0000}, "Ignitor"},
                {{10, 0x1801}, "Series 2 Ignitor"},
                {{10, 0x1C03}, "Legendary Ignitor"},
                {{11, 0x0000}, "Flameslinger"},
                {{11, 0x1801}, "Series 2 Flameslinger"},
                {{12, 0x0000}, "Zap"},
                {{12, 0x1801}, "Series 2 Zap"},
                {{13, 0x0000}, "Wham Shell"},
                {{13, 0x2206}, "LightCore Wham Shell"},
                {{14, 0x0000}, "Gill Grunt"},
                {{14, 0x1801}, "Series 2 Gill Grunt"},
                {{14, 0x2805}, "Anchors Away Gill Grunt"},
                {{14, 0x3805}, "Tidal Wave Gill Grunt"},
                {{14, 0x3810}, "Eon's Elite Gill Grunt"},
                {{15, 0x0000}, "Slam Bam"},
                {{15, 0x1801}, "Series 2 Slam Bam"},
                {{15, 0x1C03}, "Legendary Slam Bam"},
                {{15, 0x4810}, "Eon's Elite Slam Bam"},
                {{16, 0x0000}, "Spyro"},
                {{16, 0x1801}, "Series 2 Spyro"},
                {{16, 0x2C02}, "Dark Mega Ram Spyro"},
                {{16, 0x2805}, "Mega Ram Spyro"},
                {{16, 0x3810}, "Eon's Elite Spyro"},
                {{17, 0x0000}, "Voodood"},
                {{17, 0x4810}, "Eon's Elite Voodood"},
                {{18, 0x0000}, "Double Trouble"},
                {{18, 0x1801}, "Series 2 Double Trouble"},
                {{18, 0x1C02}, "Royal Double Trouble"},
                {{19, 0x0000}, "Trigger Happy"},
                {{19, 0x1801}, "Series 2 Trigger Happy"},
                {{19, 0x2C02}, "Springtime Trigger Happy"},
                {{19, 0x2805}, "Big Bang Trigger Happy"},
                {{19, 0x3810}, "Eon's Elite Trigger Happy"},
                {{20, 0x0000}, "Drobot"},
                {{20, 0x1801}, "Series 2 Drobot"},
                {{20, 0x1206}, "LightCore Drobot"},
                {{21, 0x0000}, "Drill Seargeant"},
                {{21, 0x1801}, "Series 2 Drill Seargeant"},
                {{22, 0x0000}, "Boomer"},
                {{22, 0x4810}, "Eon's Elite Boomer"},
                {{23, 0x0000}, "Wrecking Ball"},
                {{23, 0x1801}, "Series 2 Wrecking Ball"},
                {{24, 0x0000}, "Camo"},
                {{24, 0x2805}, "Thorn Horn Camo"},
                {{25, 0x0000}, "Zook"},
                {{25, 0x1801}, "Series 2 Zook"},
                {{25, 0x4810}, "Eon's Elite Zook"},
                {{26, 0x0000}, "Stealth Elf"},
                {{26, 0x1801}, "Series 2 Stealth Elf"},
                {{26, 0x2C02}, "Dark Stealth Elf"},
                {{26, 0x1C03}, "Legendary Stealth Elf"},
                {{26, 0x2805}, "Ninja Stealth Elf"},
                {{26, 0x3810}, "Eon's Elite Stealth Elf"},
                {{27, 0x0000}, "Stump Smash"},
                {{27, 0x1801}, "Series 2 Stump Smash"},
                {{28, 0x0000}, "Dark Spyro"},
                {{29, 0x0000}, "Hex"},
                {{29, 0x1801}, "Series 2 Hex"},
                {{29, 0x1206}, "LightCore Hex"},
                {{30, 0x0000}, "Chop Chop"},
                {{30, 0x1801}, "Series 2 Chop Chop"},
                {{30, 0x2805}, "Twin Blade Chop Chop"},
                {{30, 0x3810}, "Eon's Elite Chop Chop"},
                {{31, 0x0000}, "Ghost Roaster"},
                {{31, 0x4810}, "Eon's Elite Ghost Roaster"},
                {{32, 0x0000}, "Cynder"},
                {{32, 0x1801}, "Series 2 Cynder"},
                {{32, 0x2805}, "Phantom Cynder"},
                {{100, 0x0000}, "Jet Vac"},
                {{100, 0x1403}, "Legendary Jet Vac"},
                {{100, 0x2805}, "Turbo Jet Vac"},
                {{100, 0x3805}, "Full Blast Jet Vac"},
                {{100, 0x1206}, "LightCore Jet Vac"},
                {{101, 0x0000}, "Swarm"},
                {{102, 0x0000}, "Crusher"},
                {{102, 0x1602}, "Granite Crusher"},
                {{103, 0x0000}, "Flashwing"},
                {{103, 0x1402}, "Jade Flash Wing"},
                {{103, 0x2206}, "LightCore Flashwing"},
                {{104, 0x0000}, "Hot Head"},
                {{105, 0x0000}, "Hot Dog"},
                {{105, 0x1402}, "Molten Hot Dog"},
                {{105, 0x2805}, "Fire Bone Hot Dog"},
                {{106, 0x0000}, "Chill"},
                {{106, 0x1603}, "Legendary Chill"},
                {{106, 0x2805}, "Blizzard Chill"},
                {{106, 0x1206}, "LightCore Chill"},
                {{107, 0x0000}, "Thumpback"},
                {{108, 0x0000}, "Pop Fizz"},
                {{108, 0x1402}, "Punch Pop Fizz"},
                {{108, 0x3C02}, "Love Potion Pop Fizz"},
                {{108, 0x2805}, "Super Gulp Pop Fizz"},
                {{108, 0x3805}, "Fizzy Frenzy Pop Fizz"},
                {{108, 0x1206}, "LightCore Pop Fizz"},
                {{109, 0x0000}, "Ninjini"},
                {{109, 0x1602}, "Scarlet Ninjini"},
                {{110, 0x0000}, "Bouncer"},
                {{110, 0x1603}, "Legendary Bouncer"},
                {{111, 0x0000}, "Sprocket"},
                {{111, 0x2805}, "Heavy Duty Sprocket"},
                {{112, 0x0000}, "Tree Rex"},
                {{112, 0x1602}, "Gnarly Tree Rex"},
                {{113, 0x0000}, "Shroomboom"},
                {{113, 0x3805}, "Sure Shot Shroomboom"},
                {{113, 0x1206}, "LightCore Shroomboom"},
                {{114, 0x0000}, "Eye Brawl"},
                {{115, 0x0000}, "Fright Rider"},
                {{200, 0x0000}, "Anvil Rain"},
                {{201, 0x0000}, "Hidden Treasure"},
                {{201, 0x2000}, "Platinum Hidden Treasure"},
                {{202, 0x0000}, "Healing Elixir"},
                {{203, 0x0000}, "Ghost Pirate Swords"},
                {{204, 0x0000}, "Time Twist Hourglass"},
                {{205, 0x0000}, "Sky Iron Shield"},
                {{206, 0x0000}, "Winged Boots"},
                {{207, 0x0000}, "Sparx the Dragonfly"},
                {{208, 0x0000}, "Dragonfire Cannon"},
                {{208, 0x1602}, "Golden Dragonfire Cannon"},
                {{209, 0x0000}, "Scorpion Striker"},
                {{210, 0x3002}, "Biter's Bane"},
                {{210, 0x3008}, "Sorcerous Skull"},
                {{210, 0x300B}, "Axe of Illusion"},
                {{210, 0x300E}, "Arcane Hourglass"},
                {{210, 0x3012}, "Spell Slapper"},
                {{210, 0x3014}, "Rune Rocket"},
                {{211, 0x3001}, "Tidal Tiki"},
                {{211, 0x3002}, "Wet Walter"},
                {{211, 0x3006}, "Flood Flask"},
                {{211, 0x3406}, "Legendary Flood Flask"},
                {{211, 0x3007}, "Soaking Staff"},
                {{211, 0x300B}, "Aqua Axe"},
                {{211, 0x3016}, "Frost Helm"},
                {{212, 0x3003}, "Breezy Bird"},
                {{212, 0x3006}, "Drafty Decanter"},
                {{212, 0x300D}, "Tempest Timer"},
                {{212, 0x3010}, "Cloudy Cobra"},
                {{212, 0x3011}, "Storm Warning"},
                {{212, 0x3018}, "Cyclone Saber"},
                {{213, 0x3004}, "Spirit Sphere"},
                {{213, 0x3404}, "Legendary Spirit Sphere"},
                {{213, 0x3008}, "Spectral Skull"},
                {{213, 0x3408}, "Legendary Spectral Skull"},
                {{213, 0x300B}, "Haunted Hatchet"},
                {{213, 0x300C}, "Grim Gripper"},
                {{213, 0x3010}, "Spooky Snake"},
                {{213, 0x3017}, "Dream Piercer"},
                {{214, 0x3000}, "Tech Totem"},
                {{214, 0x3007}, "Automatic Angel"},
                {{214, 0x3009}, "Factory Flower"},
                {{214, 0x300C}, "Grabbing Gadget"},
                {{214, 0x3016}, "Makers Mana"},
                {{214, 0x301A}, "Topsy Techy"},
                {{215, 0x3005}, "Eternal Flame"},
                {{215, 0x3009}, "Fire Flower"},
                {{215, 0x3011}, "Scorching Stopper"},
                {{215, 0x3012}, "Searing Spinner"},
                {{215, 0x3017}, "Spark Spear"},
                {{215, 0x301B}, "Blazing Belch"},
                {{216, 0x3000}, "Banded Boulder"},
                {{216, 0x3003}, "Rock Hawk"},
                {{216, 0x300A}, "Slag Hammer"},
                {{216, 0x300E}, "Dust Of Time"},
                {{216, 0x3013}, "Spinning Sandstorm"},
                {{216, 0x301A}, "Rubble Trouble"},
                {{217, 0x3003}, "Oak Eagle"},
                {{217, 0x3005}, "Emerald Energy"},
                {{217, 0x300A}, "Weed Whacker"},
                {{217, 0x3010}, "Seed Serpent"},
                {{217, 0x3018}, "Jade Blade"},
                {{217, 0x301B}, "Shrub Shrieker"},
                {{218, 0x3000}, "Dark Dagger"},
                {{218, 0x3014}, "Shadow Spider"},
                {{218, 0x301A}, "Ghastly Grimace"},
                {{219, 0x3000}, "Shining Ship"},
                {{219, 0x300F}, "Heavenly Hawk"},
                {{219, 0x301B}, "Beam Scream"},
                {{220, 0x301E}, "Kaos Trap"},
                {{220, 0x351F}, "Ultimate Kaos Trap"},
                {{230, 0x0000}, "Hand of Fate"},
                {{230, 0x3403}, "Legendary Hand of Fate"},
                {{231, 0x0000}, "Piggy Bank"},
                {{232, 0x0000}, "Rocket Ram"},
                {{233, 0x0000}, "Tiki Speaky"},
                {{300, 0x0000}, "Dragon’s Peak"},
                {{301, 0x0000}, "Empire of Ice"},
                {{302, 0x0000}, "Pirate Seas"},
                {{303, 0x0000}, "Darklight Crypt"},
                {{304, 0x0000}, "Volcanic Vault"},
                {{305, 0x0000}, "Mirror of Mystery"},
                {{306, 0x0000}, "Nightmare Express"},
                {{307, 0x0000}, "Sunscraper Spire"},
                {{308, 0x0000}, "Midnight Museum"},
                {{404, 0x0000}, "Legendary Bash"},
                {{416, 0x0000}, "Legendary Spyro"},
                {{419, 0x0000}, "Legendary Trigger Happy"},
                {{430, 0x0000}, "Legendary Chop Chop"},
                {{450, 0x0000}, "Gusto"},
                {{451, 0x0000}, "Thunderbolt"},
                {{452, 0x0000}, "Fling Kong"},
                {{453, 0x0000}, "Blades"},
                {{453, 0x3403}, "Legendary Blades"},
                {{454, 0x0000}, "Wallop"},
                {{455, 0x0000}, "Head Rush"},
                {{455, 0x3402}, "Nitro Head Rush"},
                {{456, 0x0000}, "Fist Bump"},
                {{457, 0x0000}, "Rocky Roll"},
                {{458, 0x0000}, "Wildfire"},
                {{458, 0x3402}, "Dark Wildfire"},
                {{459, 0x0000}, "Ka Boom"},
                {{460, 0x0000}, "Trail Blazer"},
                {{461, 0x0000}, "Torch"},
                {{462, 0x3000}, "Snap Shot"},
                {{462, 0x3402}, "Dark Snap Shot"},
                {{463, 0x0000}, "Lob Star"},
                {{463, 0x3402}, "Winterfest Lob-Star"},
                {{464, 0x0000}, "Flip Wreck"},
                {{465, 0x0000}, "Echo"},
                {{466, 0x0000}, "Blastermind"},
                {{467, 0x0000}, "Enigma"},
                {{468, 0x0000}, "Deja Vu"},
                {{468, 0x3403}, "Legendary Deja Vu"},
                {{469, 0x0000}, "Cobra Candabra"},
                {{469, 0x3402}, "King Cobra Cadabra"},
                {{470, 0x0000}, "Jawbreaker"},
                {{470, 0x3403}, "Legendary Jawbreaker"},
                {{471, 0x0000}, "Gearshift"},
                {{472, 0x0000}, "Chopper"},
                {{473, 0x0000}, "Tread Head"},
                {{474, 0x0000}, "Bushwack"},
                {{474, 0x3403}, "Legendary Bushwack"},
                {{475, 0x0000}, "Tuff Luck"},
                {{476, 0x0000}, "Food Fight"},
                {{476, 0x3402}, "Dark Food Fight"},
                {{477, 0x0000}, "High Five"},
                {{478, 0x0000}, "Krypt King"},
                {{478, 0x3402}, "Nitro Krypt King"},
                {{479, 0x0000}, "Short Cut"},
                {{480, 0x0000}, "Bat Spin"},
                {{481, 0x0000}, "Funny Bone"},
                {{482, 0x0000}, "Knight Light"},
                {{483, 0x0000}, "Spotlight"},
                {{484, 0x0000}, "Knight Mare"},
                {{485, 0x0000}, "Blackout"},
                {{502, 0x0000}, "Bop"},
                {{505, 0x0000}, "Terrabite"},
                {{506, 0x0000}, "Breeze"},
                {{508, 0x0000}, "Pet Vac"},
                {{508, 0x3402}, "Power Punch Pet Vac"},
                {{507, 0x0000}, "Weeruptor"},
                {{507, 0x3402}, "Eggcellent Weeruptor"},
                {{509, 0x0000}, "Small Fry"},
                {{510, 0x0000}, "Drobit"},
                {{519, 0x0000}, "Trigger Snappy"},
                {{526, 0x0000}, "Whisper Elf"},
                {{540, 0x0000}, "Barkley"},
                {{540, 0x3402}, "Gnarly Barkley"},
                {{541, 0x0000}, "Thumpling"},
                {{514, 0x0000}, "Gill Runt"},
                {{542, 0x0000}, "Mini-Jini"},
                {{503, 0x0000}, "Spry"},
                {{504, 0x0000}, "Hijinx"},
                {{543, 0x0000}, "Eye Small"},
                {{601, 0x0000}, "King Pen"},
                {{602, 0x0000}, "Tri-Tip"},
                {{603, 0x0000}, "Chopscotch"},
                {{604, 0x0000}, "Boom Bloom"},
                {{605, 0x0000}, "Pit Boss"},
                {{606, 0x0000}, "Barbella"},
                {{607, 0x0000}, "Air Strike"},
                {{608, 0x0000}, "Ember"},
                {{609, 0x0000}, "Ambush"},
                {{610, 0x0000}, "Dr. Krankcase"},
                {{611, 0x0000}, "Hood Sickle"},
                {{612, 0x0000}, "Tae Kwon Crow"},
                {{613, 0x0000}, "Golden Queen"},
                {{614, 0x0000}, "Wolfgang"},
                {{615, 0x0000}, "Pain-Yatta"},
                {{616, 0x0000}, "Mysticat"},
                {{617, 0x0000}, "Starcast"},
                {{618, 0x0000}, "Buckshot"},
                {{619, 0x0000}, "Aurora"},
                {{620, 0x0000}, "Flare Wolf"},
                {{621, 0x0000}, "Chompy Mage"},
                {{622, 0x0000}, "Bad Juju"},
                {{623, 0x0000}, "Grave Clobber"},
                {{624, 0x0000}, "Blaster-Tron"},
                {{625, 0x0000}, "Ro-Bow"},
                {{626, 0x0000}, "Chain Reaction"},
                {{627, 0x0000}, "Kaos"},
                {{628, 0x0000}, "Wild Storm"},
                {{629, 0x0000}, "Tidepool"},
                {{630, 0x0000}, "Crash Bandicoot"},
                {{631, 0x0000}, "Dr. Neo Cortex"},
                {{1000, 0x0000}, "Boom Jet (Bottom)"},
                {{1001, 0x0000}, "Free Ranger (Bottom)"},
                {{1001, 0x2403}, "Legendary Free Ranger (Bottom)"},
                {{1002, 0x0000}, "Rubble Rouser (Bottom)"},
                {{1003, 0x0000}, "Doom Stone (Bottom)"},
                {{1004, 0x0000}, "Blast Zone (Bottom)"},
                {{1004, 0x2402}, "Dark Blast Zone (Bottom)"},
                {{1005, 0x0000}, "Fire Kraken (Bottom)"},
                {{1005, 0x2402}, "Jade Fire Kraken (Bottom)"},
                {{1006, 0x0000}, "Stink Bomb (Bottom)"},
                {{1007, 0x0000}, "Grilla Drilla (Bottom)"},
                {{1008, 0x0000}, "Hoot Loop (Bottom)"},
                {{1008, 0x2402}, "Enchanted Hoot Loop (Bottom)"},
                {{1009, 0x0000}, "Trap Shadow (Bottom)"},
                {{1010, 0x0000}, "Magna Charge (Bottom)"},
                {{1010, 0x2402}, "Nitro Magna Charge (Bottom)"},
                {{1011, 0x0000}, "Spy Rise (Bottom)"},
                {{1012, 0x0000}, "Night Shift (Bottom)"},
                {{1012, 0x2403}, "Legendary Night Shift (Bottom)"},
                {{1013, 0x0000}, "Rattle Shake (Bottom)"},
                {{1013, 0x2402}, "Quick Draw Rattle Shake (Bottom)"},
                {{1014, 0x0000}, "Freeze Blade (Bottom)"},
                {{1014, 0x2402}, "Nitro Freeze Blade (Bottom)"},
                {{1015, 0x0000}, "Wash Buckler (Bottom)"},
                {{1015, 0x2402}, "Dark Wash Buckler (Bottom)"},
                {{2000, 0x0000}, "Boom Jet (Top)"},
                {{2001, 0x0000}, "Free Ranger (Top)"},
                {{2001, 0x2403}, "Legendary Free Ranger (Top)"},
                {{2002, 0x0000}, "Rubble Rouser (Top)"},
                {{2003, 0x0000}, "Doom Stone (Top)"},
                {{2004, 0x0000}, "Blast Zone (Top)"},
                {{2004, 0x2402}, "Dark Blast Zone (Top)"},
                {{2005, 0x0000}, "Fire Kraken (Top)"},
                {{2005, 0x2402}, "Jade Fire Kraken (Top)"},
                {{2006, 0x0000}, "Stink Bomb (Top)"},
                {{2007, 0x0000}, "Grilla Drilla (Top)"},
                {{2008, 0x0000}, "Hoot Loop (Top)"},
                {{2008, 0x2402}, "Enchanted Hoot Loop (Top)"},
                {{2009, 0x0000}, "Trap Shadow (Top)"},
                {{2010, 0x0000}, "Magna Charge (Top)"},
                {{2010, 0x2402}, "Nitro Magna Charge (Top)"},
                {{2011, 0x0000}, "Spy Rise (Top)"},
                {{2012, 0x0000}, "Night Shift (Top)"},
                {{2012, 0x2403}, "Legendary Night Shift (Top)"},
                {{2013, 0x0000}, "Rattle Shake (Top)"},
                {{2013, 0x2402}, "Quick Draw Rattle Shake (Top)"},
                {{2014, 0x0000}, "Freeze Blade (Top)"},
                {{2014, 0x2402}, "Nitro Freeze Blade (Top)"},
                {{2015, 0x0000}, "Wash Buckler (Top)"},
                {{2015, 0x2402}, "Dark Wash Buckler (Top)"},
                {{3000, 0x0000}, "Scratch"},
                {{3001, 0x0000}, "Pop Thorn"},
                {{3002, 0x0000}, "Slobber Tooth"},
                {{3002, 0x2402}, "Dark Slobber Tooth"},
                {{3003, 0x0000}, "Scorp"},
                {{3004, 0x0000}, "Fryno"},
                {{3004, 0x3805}, "Hog Wild Fryno"},
                {{3005, 0x0000}, "Smolderdash"},
                {{3005, 0x2206}, "LightCore Smolderdash"},
                {{3006, 0x0000}, "Bumble Blast"},
                {{3006, 0x2402}, "Jolly Bumble Blast"},
                {{3006, 0x2206}, "LightCore Bumble Blast"},
                {{3007, 0x0000}, "Zoo Lou"},
                {{3007, 0x2403}, "Legendary Zoo Lou"},
                {{3008, 0x0000}, "Dune Bug"},
                {{3009, 0x0000}, "Star Strike"},
                {{3009, 0x2602}, "Enchanted Star Strike"},
                {{3009, 0x2206}, "LightCore Star Strike"},
                {{3010, 0x0000}, "Countdown"},
                {{3010, 0x2402}, "Kickoff Countdown"},
                {{3010, 0x2206}, "LightCore Countdown"},
                {{3011, 0x0000}, "Wind Up"},
                {{3011, 0x2404}, "Gear Head VVind Up"},
                {{3012, 0x0000}, "Roller Brawl"},
                {{3013, 0x0000}, "Grim Creeper"},
                {{3013, 0x2603}, "Legendary Grim Creeper"},
                {{3013, 0x2206}, "LightCore Grim Creeper"},
                {{3014, 0x0000}, "Rip Tide"},
                {{3015, 0x0000}, "Punk Shock"},
                {{3200, 0x0000}, "Battle Hammer"},
                {{3201, 0x0000}, "Sky Diamond"},
                {{3202, 0x0000}, "Platinum Sheep"},
                {{3203, 0x0000}, "Groove Machine"},
                {{3204, 0x0000}, "UFO Hat"},
                {{3300, 0x0000}, "Sheep Wreck Island"},
                {{3301, 0x0000}, "Tower of Time"},
                {{3302, 0x0000}, "Fiery Forge"},
                {{3303, 0x0000}, "Arkeyan Crossbow"},
                {{3220, 0x0000}, "Jet Stream"},
                {{3221, 0x0000}, "Tomb Buggy"},
                {{3222, 0x0000}, "Reef Ripper"},
                {{3223, 0x0000}, "Burn Cycle"},
                {{3224, 0x0000}, "Hot Streak"},
                {{3224, 0x4402}, "Dark Hot Streak"},
                {{3224, 0x4004}, "E3 Hot Streak"},
                {{3224, 0x441E}, "Golden Hot Streak"},
                {{3225, 0x0000}, "Shark Tank"},
                {{3226, 0x0000}, "Thump Truck"},
                {{3227, 0x0000}, "Crypt Crusher"},
                {{3228, 0x0000}, "Stealth Stinger"},
                {{3228, 0x4402}, "Nitro Stealth Stinger"},
                {{3231, 0x0000}, "Dive Bomber"},
                {{3231, 0x4402}, "Spring Ahead Dive Bomber"},
                {{3232, 0x0000}, "Sky Slicer"},
                {{3233, 0x0000}, "Clown Cruiser (Nintendo Only)"},
                {{3233, 0x4402}, "Dark Clown Cruiser (Nintendo Only)"},
                {{3234, 0x0000}, "Gold Rusher"},
                {{3234, 0x4402}, "Power Blue Gold Rusher"},
                {{3235, 0x0000}, "Shield Striker"},
                {{3236, 0x0000}, "Sun Runner"},
                {{3236, 0x4403}, "Legendary Sun Runner"},
                {{3237, 0x0000}, "Sea Shadow"},
                {{3237, 0x4402}, "Dark Sea Shadow"},
                {{3238, 0x0000}, "Splatter Splasher"},
                {{3238, 0x4402}, "Power Blue Splatter Splasher"},
                {{3239, 0x0000}, "Soda Skimmer"},
                {{3239, 0x4402}, "Nitro Soda Skimmer"},
                {{3240, 0x0000}, "Barrel Blaster (Nintendo Only)"},
                {{3240, 0x4402}, "Dark Barrel Blaster (Nintendo Only)"},
                {{3241, 0x0000}, "Buzz Wing"},
                {{3400, 0x0000}, "Fiesta"},
                {{3400, 0x4515}, "Frightful Fiesta"},
                {{3401, 0x0000}, "High Volt"},
                {{3402, 0x0000}, "Splat"},
                {{3402, 0x4502}, "Power Blue Splat"},
                {{3406, 0x0000}, "Stormblade"},
                {{3411, 0x0000}, "Smash Hit"},
                {{3411, 0x4502}, "Steel Plated Smash Hit"},
                {{3412, 0x0000}, "Spitfire"},
                {{3412, 0x4502}, "Dark Spitfire"},
                {{3413, 0x0000}, "Hurricane Jet Vac"},
                {{3413, 0x4503}, "Legendary Hurricane Jet Vac"},
                {{3414, 0x0000}, "Double Dare Trigger Happy"},
                {{3414, 0x4502}, "Power Blue Double Dare Trigger Happy"},
                {{3415, 0x0000}, "Super Shot Stealth Elf"},
                {{3415, 0x4502}, "Dark Super Shot Stealth Elf"},
                {{3416, 0x0000}, "Shark Shooter Terrafin"},
                {{3417, 0x0000}, "Bone Bash Roller Brawl"},
                {{3417, 0x4503}, "Legendary Bone Bash Roller Brawl"},
                {{3420, 0x0000}, "Big Bubble Pop Fizz"},
                {{3420, 0x450E}, "Birthday Bash Big Bubble Pop Fizz"},
                {{3421, 0x0000}, "Lava Lance Eruptor"},
                {{3422, 0x0000}, "Deep Dive Gill Grunt"},
                {{3423, 0x0000}, "Turbo Charge Donkey Kong (Nintendo Only)"},
                {{3423, 0x4502}, "Dark Turbo Charge Donkey Kong (Nintendo Only)"},
                {{3424, 0x0000}, "Hammer Slam Bowser (Nintendo Only)"},
                {{3424, 0x4502}, "Dark Hammer Slam Bowser (Nintendo Only)"},
                {{3425, 0x0000}, "Dive-Clops"},
                {{3425, 0x450E}, "Missile-Tow Dive-Clops"},
                {{3426, 0x0000}, "Astroblast"},
                {{3426, 0x4503}, "Legendary Astroblast"},
                {{3427, 0x0000}, "Nightfall"},
                {{3428, 0x0000}, "Thrillipede"},
                {{3428, 0x450D}, "Eggcited Thrillipede"},
                {{3500, 0x0000}, "Sky Trophy"},
                {{3501, 0x0000}, "Land Trophy"},
                {{3502, 0x0000}, "Sea Trophy"},
                {{3503, 0x0000}, "Kaos Trophy"},
};


SkylanderUSBDevice::SkylanderUSBDevice() : Device(0x3014, 0x5001, 0, 0, 0, 0x0040, 0x0040) {
}

SkylanderUSBDevice::~SkylanderUSBDevice() = default;

bool SkylanderUSBDevice::GetDescriptor(uint8_t descType,
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
    *(uint8_t *) (currentWritePtr + 2) = 0x11; // bcdHID
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
    *(uint8_t *) (currentWritePtr + 4) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;
    // endpoint descriptor 2
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x02; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;

    memcpy(buffer, configurationDescriptor,
           std::min<uint32_t>(bufferLength, sizeof(configurationDescriptor)));

    return true;
}

bool SkylanderUSBDevice::SetDescriptor(uint8_t descType,
                                       uint8_t descIndex,
                                       uint16_t lang,
                                       uint8_t *buffer,
                                       uint32_t bufferLength) {
    return true;
}

bool SkylanderUSBDevice::GetReport(uint8_t *buffer,
                                   uint32_t bufferLength) {
    return true;
}

bool SkylanderUSBDevice::SetReport(uint8_t *buffer,
                                   uint32_t bufferLength) {
    g_skyportal.ControlTransfer(buffer, bufferLength);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

bool SkylanderUSBDevice::GetIdle(uint8_t ifIndex,
                                 uint8_t reportId,
                                 uint8_t *duration) {
    return true;
}

bool SkylanderUSBDevice::SetIdle(uint8_t ifIndex,
                                 uint8_t reportId,
                                 uint8_t duration) {
    return true;
}

bool SkylanderUSBDevice::GetProtocol(uint8_t ifIndex,
                                     uint8_t *protocol) {
    return true;
}

bool SkylanderUSBDevice::SetProtocol(uint8_t ifIndex,
                                     uint8_t protocol) {
    return true;
}

bool SkylanderUSBDevice::Read(uint8_t *buffer,
                              uint32_t bufferLength) {
    const std::array<uint8_t, 64> response = g_skyportal.GetStatus();
    memcpy(buffer, response.data(), bufferLength);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    return true;
}

bool SkylanderUSBDevice::Write(uint8_t *buffer,
                               uint32_t bufferLength) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

void SkylanderPortal::ControlTransfer(uint8_t *buf, uint32_t length) {
    std::array<uint8_t, 64> interruptResponse = {};
    switch (buf[0]) {
        case 'A': {
            interruptResponse = {buf[0], buf[1], 0xFF, 0x77};
            g_skyportal.Activate();
            break;
        }
        case 'C': {
            g_skyportal.SetLeds(0x01, buf[1], buf[2], buf[3]);
            break;
        }
        case 'J': {
            g_skyportal.SetLeds(buf[1], buf[2], buf[3], buf[4]);
            interruptResponse = {buf[0]};
            break;
        }
        case 'L': {
            uint8_t side = buf[1];
            if (side == 0x02) {
                side = 0x04;
            }
            g_skyportal.SetLeds(side, buf[2], buf[3], buf[4]);
            break;
        }
        case 'M': {
            interruptResponse = {buf[0], buf[1], 0x00, 0x19};
            break;
        }
        case 'Q': {
            const uint8_t skyNum = buf[1] & 0xF;
            const uint8_t block  = buf[2];
            g_skyportal.QueryBlock(skyNum, block, interruptResponse.data());
            break;
        }
        case 'R': {
            interruptResponse = {buf[0], 0x02, 0x1b};
            break;
        }
        case 'S':
        case 'V': {
            // No response needed
            break;
        }
        case 'W': {
            const uint8_t skyNum = buf[1] & 0xF;
            const uint8_t block  = buf[2];
            g_skyportal.WriteBlock(skyNum, block, &buf[3], interruptResponse.data());
            break;
        }
        default:
            break;
    }
    if (interruptResponse[0] != 0) {
        std::lock_guard lock(m_queryMutex);
        m_queries.push(interruptResponse);
    }
}

std::array<uint8_t, 64> SkylanderPortal::GetStatus() {
    std::lock_guard lock(m_queryMutex);
    std::array<uint8_t, 64> interruptResponse = {};

    if (!m_queries.empty()) {
        interruptResponse = m_queries.front();
        m_queries.pop();
        // This needs to happen after ~22 milliseconds
    } else {
        uint32_t status = 0;
        uint8_t active  = 0x00;
        if (m_activated) {
            active = 0x01;
        }

        for (int i = MAX_SKYLANDERS - 1; i >= 0; i--) {
            auto &s = m_skylanders[i];

            if (!s.queuedStatus.empty()) {
                s.status = s.queuedStatus.front();
                s.queuedStatus.pop();
            }
            status <<= 2;
            status |= s.status;
        }
        interruptResponse = {0x53, uint8_t(status & 0xFF), uint8_t((status >> 8) & 0xFF), uint8_t((status >> 16) & 0xFF), uint8_t((status >> 24) & 0xFF), m_interruptCounter++,
                             active, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00};
    }
    return interruptResponse;
}

void SkylanderPortal::Activate() {
    std::lock_guard lock(m_skyMutex);
    if (m_activated) {
        // If the portal was already active no change is needed
        return;
    }

    // If not we need to advertise change to all the figures present on the portal
    for (auto &s : m_skylanders) {
        if (s.status & 1) {
            s.queuedStatus.push(3);
            s.queuedStatus.push(1);
        }
    }

    m_activated = true;
}

void SkylanderPortal::Deactivate() {
    std::lock_guard lock(m_skyMutex);

    for (auto &s : m_skylanders) {
        // check if at the end of the updates there would be a figure on the portal
        if (!s.queuedStatus.empty()) {
            s.status       = s.queuedStatus.back();
            s.queuedStatus = std::queue<uint8_t>();
        }

        s.status &= 1;
    }

    m_activated = false;
}

void SkylanderPortal::SetLeds(uint8_t side, uint8_t r, uint8_t g, uint8_t b) {
    std::lock_guard lock(m_skyMutex);
    if (side == 0x00) {
        m_colorRight.red   = r;
        m_colorRight.green = g;
        m_colorRight.blue  = b;
    } else if (side == 0x01) {
        m_colorRight.red   = r;
        m_colorRight.green = g;
        m_colorRight.blue  = b;

        m_colorLeft.red   = r;
        m_colorLeft.green = g;
        m_colorLeft.blue  = b;
    } else if (side == 0x02) {
        m_colorLeft.red   = r;
        m_colorLeft.green = g;
        m_colorLeft.blue  = b;
    } else if (side == 0x03) {
        m_colorTrap.red   = r;
        m_colorTrap.green = g;
        m_colorTrap.blue  = b;
    }
}

void SkylanderPortal::QueryBlock(uint8_t skyNum, uint8_t block, uint8_t *replyBuf) {
    std::lock_guard lock(m_skyMutex);

    const auto &skylander = m_skylanders[skyNum];

    replyBuf[0] = 'Q';
    replyBuf[2] = block;
    if (skylander.status & 1) {
        replyBuf[1] = (0x10 | skyNum);
        memcpy(replyBuf + 3, skylander.data.data() + (16 * block), 16);
    } else {
        replyBuf[1] = skyNum;
    }
}

void SkylanderPortal::WriteBlock(uint8_t skyNum, uint8_t block,
                                 const uint8_t *toWriteBuf, uint8_t *replyBuf) {
    std::lock_guard lock(m_skyMutex);

    auto &skylander = m_skylanders[skyNum];

    replyBuf[0] = 'W';
    replyBuf[2] = block;

    if (skylander.status & 1) {
        replyBuf[1] = (0x10 | skyNum);
        memcpy(skylander.data.data() + (block * SKY_BLOCK_SIZE), toWriteBuf, SKY_BLOCK_SIZE);
        skylander.Save();
    } else {
        replyBuf[1] = skyNum;
    }
}

bool SkylanderPortal::LoadSkylander(uint8_t *buf, FILE *file, uint8_t uiSlot) {
    if (m_skylanderUIPositions[uiSlot]) {
        RemoveSkylander(uiSlot);
    }
    m_skylanderUIPositions[uiSlot] = LoadSkylander(buf, std::move(file));
    return true;
}

uint8_t SkylanderPortal::LoadSkylander(uint8_t *buf, FILE *file) {
    std::lock_guard lock(m_skyMutex);

    uint32_t skySerial = 0;
    for (int i = 3; i > -1; i--) {
        skySerial <<= 8;
        skySerial |= buf[i];
    }
    uint8_t foundSlot = 0xFF;

    // mimics spot retaining on the portal
    for (auto i = 0; i < MAX_SKYLANDERS; i++) {
        if ((m_skylanders[i].status & 1) == 0) {
            if (m_skylanders[i].lastId == skySerial) {
                foundSlot = i;
                break;
            }

            if (i < foundSlot) {
                foundSlot = i;
            }
        }
    }
    if (foundSlot != 0xFF) {
        auto &skylander = m_skylanders[foundSlot];
        memcpy(skylander.data.data(), buf, skylander.data.size());
        skylander.skyFile = std::move(file);
        skylander.status  = Skylander::ADDED;
        skylander.queuedStatus.push(Skylander::ADDED);
        skylander.queuedStatus.push(Skylander::READY);
        skylander.lastId = skySerial;
    }
    return foundSlot;
}

bool SkylanderPortal::RemoveSkylander(uint8_t skyNum) {
    if (!m_skylanderUIPositions[skyNum]) {
        return false;
    }
    DEBUG_FUNCTION_LINE("Removing Skylander %d", m_skylanderUIPositions[skyNum].value());
    std::lock_guard lock(m_skyMutex);
    auto &thesky = m_skylanders[m_skylanderUIPositions[skyNum].value()];

    if (thesky.status & Skylander::READY) {
        thesky.status = Skylander::REMOVING;
        thesky.queuedStatus.push(Skylander::REMOVING);
        thesky.queuedStatus.push(Skylander::REMOVED);
        thesky.Save();
        fclose(thesky.skyFile);
        m_skylanderUIPositions[skyNum] = std::nullopt;
        return true;
    }

    return false;
}

std::string SkylanderPortal::FindSkylander(uint16_t skyId, uint16_t skyVar) {
    for (const auto &it : GetListSkylanders()) {
        if (it.first.first == skyId && it.first.second == skyVar) {
            return it.second;
        }
    }
    return std::format("Unknown ({} {})", skyId, skyVar);
}

std::map<const std::pair<const uint16_t, const uint16_t>, const char *> SkylanderPortal::GetListSkylanders() {
    return s_listSkylanders;
}

std::string SkylanderPortal::GetSkylanderFromUISlot(uint8_t uiSlot) {
    if (m_skylanderUIPositions[uiSlot]) {
        auto &thesky    = m_skylanders[m_skylanderUIPositions[uiSlot].value()];
        uint16_t skyId  = uint16_t(thesky.data[0x11]) << 8 | uint16_t(thesky.data[0x10]);
        uint16_t skyVar = uint16_t(thesky.data[0x1D]) << 8 | uint16_t(thesky.data[0x1C]);
        return FindSkylander(skyId, skyVar);
    } else {
        return "None";
    }
}

void SkylanderPortal::Skylander::Save() {
    if (!skyFile)
        return;

    fseeko(skyFile, 0, SEEK_SET);
    fwrite(data.data(), data.size(), sizeof(data[0]), skyFile);
}
