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
                {{0, 0x1801}, "Whirlwind (S2)"},
                {{0, 0x1812}, "Whirlwind (Stone)"},
                {{0, 0x1C02}, "Whirlwind (Polar)"},
                {{0, 0x2805}, "Whirlwind (Horn Blast)"},
                {{0, 0x3810}, "Whirlwind (Eon's Elite)"},
                {{1, 0x0000}, "Sonic Boom"},
                {{1, 0x1801}, "Sonic Boom (S2)"},
                {{1, 0x1811}, "Sonic Boom (Glow in the Dark)"},
                {{1, 0x1813}, "Sonic Boom (Sparkle)"},
                {{2, 0x0000}, "Warnado"},
                {{2, 0x2206}, "Warnado (LightCore)"},
                {{3, 0x0000}, "Lightning Rod"},
                {{3, 0x1801}, "Lightning Rod (S2)"},
                {{3, 0x1814}, "Lightning Rod (Metallic Purple)"},
                {{4, 0x0000}, "Bash"},
                {{4, 0x1801}, "Bash (S2)"},
                {{5, 0x0000}, "Terrafin"},
                {{5, 0x1801}, "Terrafin (S2)"},
                {{5, 0x2805}, "Terrafin (Knockout)"},
                {{5, 0x3810}, "Terrafin (Eon's Elite)"},
                {{6, 0x0000}, "Dino-Rang"},
                {{6, 0x4810}, "Dino-Rang (Eon's Elite)"},
                {{7, 0x0000}, "Prism Break"},
                {{7, 0x1206}, "Prism Break (LightCore)"},
                {{7, 0x1214}, "Prism Break (White Flocked LightCore)"},
                {{7, 0x1801}, "Prism Break (S2)"},
                {{7, 0x2805}, "Prism Break (Hyper Beam)"},
                {{8, 0x0000}, "Sunburn"},
                {{9, 0x0000}, "Eruptor"},
                {{9, 0x1206}, "Eruptor (LightCore)"},
                {{9, 0x1801}, "Eruptor (S2)"},
                {{9, 0x1814}, "Eruptor (White Flocked)"},
                {{9, 0x2805}, "Eruptor (Lava Barf)"},
                {{9, 0x2C02}, "Eruptor (Volcanic Lava Barf)"},
                {{9, 0x3810}, "Eruptor (Eon's Elite)"},
                {{10, 0x0000}, "Ignitor"},
                {{10, 0x1801}, "Ignitor (S2)"},
                {{10, 0x1C03}, "Ignitor (Legendary)"},
                {{11, 0x0000}, "Flameslinger"},
                {{11, 0x1801}, "Flameslinger (S2)"},
                {{11, 0x1802}, "Flameslinger (Golden)"},
                {{12, 0x0000}, "Zap"},
                {{12, 0x1801}, "Zap (S2)"},
                {{13, 0x0000}, "Wham-Shell"},
                {{13, 0x2206}, "Wham-Shell (LightCore)"},
                {{14, 0x0000}, "Gill Grunt"},
                {{14, 0x1801}, "Gill Grunt (S2)"},
                {{14, 0x1817}, "Gill Grunt (Metallic Green)"},
                {{14, 0x2805}, "Gill Grunt (Anchors Away)"},
                {{14, 0x3809}, "Gill Grunt (Tidal Wave)"},
                {{14, 0x3810}, "Gill Grunt (Eon's Elite)"},
                {{15, 0x0000}, "Slam Bam"},
                {{15, 0x1801}, "Slam Bam (S2)"},
                {{15, 0x1C03}, "Slam Bam (Legendary)"},
                {{15, 0x3810}, "Slam Bam (Eon's Elite)"},
                {{16, 0x0000}, "Spyro"},
                {{16, 0x1801}, "Spyro (S2)"},
                {{16, 0x2805}, "Spyro (Mega Ram)"},
                {{16, 0x2C02}, "Spyro (Dark Mega Ram)"},
                {{16, 0x3810}, "Spyro (Eon's Elite)"},
                {{17, 0x0000}, "Voodood"},
                {{17, 0x3810}, "Voodood (Eon's Elite)"},
                {{18, 0x0000}, "Double Trouble"},
                {{18, 0x1801}, "Double Trouble (S2)"},
                {{18, 0x1C02}, "Double Trouble (Royal)"},
                {{19, 0x0000}, "Trigger Happy"},
                {{19, 0x1801}, "Trigger Happy (S2)"},
                {{19, 0x2805}, "Trigger Happy (Big Bang)"},
                {{19, 0x2C02}, "Trigger Happy (Springtime Big Bang)"},
                {{19, 0x3810}, "Trigger Happy (Eon's Elite)"},
                {{20, 0x0000}, "Drobot"},
                {{20, 0x1206}, "Drobot (LightCore)"},
                {{20, 0x1801}, "Drobot (S2)"},
                {{21, 0x0000}, "Drill Sergeant"},
                {{21, 0x1801}, "Drill Sergeant (S2)"},
                {{22, 0x0000}, "Boomer"},
                {{22, 0x4810}, "Boomer (Eon's Elite)"},
                {{23, 0x0000}, "Wrecking Ball"},
                {{23, 0x1801}, "Wrecking Ball (S2)"},
                {{23, 0x1814}, "Wrecking Ball (Metallic Purple)"},
                {{24, 0x0000}, "Camo"},
                {{24, 0x2805}, "Camo (Thorn Horn)"},
                {{25, 0x0000}, "Zook"},
                {{25, 0x1801}, "Zook (S2)"},
                {{25, 0x1812}, "Zook (Stone)"},
                {{25, 0x3810}, "Zook (Eon's Elite)"},
                {{26, 0x0000}, "Stealth Elf"},
                {{26, 0x1801}, "Stealth Elf (S2)"},
                {{26, 0x1C03}, "Stealth Elf (Legendary)"},
                {{26, 0x2805}, "Stealth Elf (Ninja)"},
                {{26, 0x2C02}, "Stealth Elf (Dark Ninja)"},
                {{26, 0x3810}, "Stealth Elf (Eon's Elite)"},
                {{27, 0x0000}, "Stump Smash"},
                {{27, 0x1801}, "Stump Smash (S2)"},
                {{27, 0x1814}, "Stump Smash (White Flocked)"},
                {{28, 0x0000}, "Spyro (Dark)"},
                {{29, 0x0000}, "Hex"},
                {{29, 0x1206}, "Hex (LightCore)"},
                {{29, 0x1801}, "Hex (S2)"},
                {{30, 0x0000}, "Chop Chop"},
                {{30, 0x1801}, "Chop Chop (S2)"},
                {{30, 0x1804}, "Chop Chop (Metallic Blue)"},
                {{30, 0x2805}, "Chop Chop (Twin Blade)"},
                {{30, 0x2816}, "Chop Chop (Green Twin Blade)"},
                {{30, 0x3810}, "Chop Chop (Eon's Elite)"},
                {{31, 0x0000}, "Ghost Roaster"},
                {{31, 0x4810}, "Ghost Roaster (Eon's Elite)"},
                {{32, 0x0000}, "Cynder"},
                {{32, 0x1801}, "Cynder (S2)"},
                {{32, 0x1811}, "Cynder (Glow in the Dark)"},
                {{32, 0x2805}, "Cynder (Phantom)"},
                {{100, 0x1000}, "Jet-Vac"},
                {{100, 0x1206}, "Jet-Vac (LightCore)"},
                {{100, 0x1403}, "Jet-Vac (Legendary)"},
                {{100, 0x2805}, "Jet-Vac (Turbo)"},
                {{100, 0x3805}, "Jet-Vac (Full Blast)"},
                {{101, 0x1206}, "Swarm"},
                {{102, 0x1206}, "Crusher"},
                {{102, 0x1602}, "Crusher (Granite)"},
                {{103, 0x1000}, "Flashwing"},
                {{103, 0x1402}, "Flashwing (Jade)"},
                {{103, 0x2206}, "Flashwing (LightCore)"},
                {{104, 0x1206}, "Hot Head"},
                {{104, 0x1213}, "Hot Head (Sparkle)"},
                {{105, 0x1000}, "Hot Dog"},
                {{105, 0x1015}, "Hot Dog (Bronze)"},
                {{105, 0x1402}, "Hot Dog (Molten)"},
                {{105, 0x2805}, "Hot Dog (Fire Bone)"},
                {{105, 0x281A}, "Hot Dog (Red Flame Frito-Lay Fire Bone)"},
                {{105, 0x281B}, "Hot Dog (Green Flame Frito-Lay Fire Bone)"},
                {{105, 0x281C}, "Hot Dog (Purple Flame Frito-Lay Fire Bone)"},
                {{106, 0x1000}, "Chill"},
                {{106, 0x1206}, "Chill (LightCore)"},
                {{106, 0x1603}, "Chill (Legendary LightCore)"},
                {{106, 0x2805}, "Chill (Blizzard)"},
                {{107, 0x1206}, "Thumpback"},
                {{108, 0x1000}, "Pop Fizz"},
                {{108, 0x1206}, "Pop Fizz (LightCore)"},
                {{108, 0x1402}, "Pop Fizz (Punch)"},
                {{108, 0x2805}, "Pop Fizz (Super Gulp)"},
                {{108, 0x3805}, "Pop Fizz (Fizzy Frenzy)"},
                {{108, 0x3C02}, "Pop Fizz (Love Potion Fizzy Frenzy)"},
                {{109, 0x1206}, "Ninjini"},
                {{109, 0x1602}, "Ninjini (Scarlet)"},
                {{110, 0x1206}, "Bouncer"},
                {{110, 0x1603}, "Bouncer (Legendary)"},
                {{111, 0x1000}, "Sprocket"},
                {{111, 0x1013}, "Sprocket (Sparkle)"},
                {{111, 0x2805}, "Sprocket (Heavy Duty)"},
                {{111, 0x2819}, "Sprocket (Heavy Metal Heavy Duty)"},
                {{112, 0x1206}, "Tree Rex"},
                {{112, 0x1602}, "Tree Rex (Gnarly)"},
                {{113, 0x1000}, "Shroomboom"},
                {{113, 0x1017}, "Shroomboom (Metallic Green)"},
                {{113, 0x1206}, "Shroomboom (LightCore)"},
                {{113, 0x3801}, "Shroomboom (Sure Shot)"},
                {{114, 0x1206}, "Eye-Brawl"},
                {{114, 0x1214}, "Eye-Brawl (Metallic Purple)"},
                {{114, 0x1215}, "Eye-Brawl (Pumpkin)"},
                {{115, 0x1000}, "Fright Rider"},
                {{115, 0x1011}, "Fright Rider (Glow in the Dark)"},
                {{115, 0x1015}, "Fright Rider (Halloween)"},
                {{200, 0x0000}, "Anvil Rain"},
                {{201, 0x0000}, "Hidden Treasure"},
                {{201, 0x0002}, "Hidden Treasure (Platinum)"},
                {{202, 0x0000}, "Healing Elixir"},
                {{203, 0x0000}, "Ghost Pirate Swords"},
                {{204, 0x0000}, "Time Twister Hourglass"},
                {{205, 0x0000}, "Sky-Iron Shield"},
                {{206, 0x0000}, "Winged Boots"},
                {{207, 0x0000}, "Sparx Dragonfly"},
                {{208, 0x1206}, "Dragonfire Cannon"},
                {{208, 0x1602}, "Golden Dragonfire Cannon"},
                {{209, 0x1206}, "Scorpion Striker Catapult"},
                {{210, 0x3002}, "Magic Log Holder"},
                {{210, 0x3008}, "Magic Skull"},
                {{210, 0x300B}, "Magic Axe"},
                {{210, 0x300E}, "Magic Hourglass"},
                {{210, 0x3012}, "Magic Totem"},
                {{210, 0x3015}, "Magic Rocket"},
                {{211, 0x3001}, "Water Tiki"},
                {{211, 0x3002}, "Water Log Holder"},
                {{211, 0x3006}, "Water Jughead"},
                {{211, 0x3007}, "Water Angel"},
                {{211, 0x300B}, "Water Axe"},
                {{211, 0x3016}, "Water Flying Helmet"},
                {{211, 0x3406}, "Water Jughead (Legendary)"},
                {{212, 0x3003}, "Air Toucan"},
                {{212, 0x3006}, "Air Jughead"},
                {{212, 0x300E}, "Air Hourglass"},
                {{212, 0x3010}, "Air Snake"},
                {{212, 0x3011}, "Air Screamer"},
                {{212, 0x3018}, "Air Sword"},
                {{213, 0x3004}, "Undead Orb"},
                {{213, 0x3008}, "Undead Skull"},
                {{213, 0x300B}, "Undead Axe"},
                {{213, 0x300C}, "Undead Hand"},
                {{213, 0x3010}, "Undead Snake"},
                {{213, 0x3017}, "Undead Captain's Hat"},
                {{213, 0x3404}, "Undead Orb (Legendary)"},
                {{213, 0x3408}, "Undead Skull (Legendary)"},
                {{214, 0x3001}, "Tech Tiki"},
                {{214, 0x3007}, "Tech Angel"},
                {{214, 0x3009}, "Tech Scepter"},
                {{214, 0x300C}, "Tech Hand"},
                {{214, 0x3016}, "Tech Flying Helmet"},
                {{214, 0x301A}, "Tech Handstand"},
                {{215, 0x3005}, "Fire Torch"},
                {{215, 0x3009}, "Fire Scepter"},
                {{215, 0x3011}, "Fire Screamer"},
                {{215, 0x3012}, "Fire Totem"},
                {{215, 0x3017}, "Fire Captain's Hat"},
                {{215, 0x301B}, "Fire Yawn"},
                {{216, 0x3003}, "Earth Toucan"},
                {{216, 0x3004}, "Earth Orb"},
                {{216, 0x300A}, "Earth Hammer"},
                {{216, 0x300E}, "Earth Hourglass"},
                {{216, 0x3012}, "Earth Totem"},
                {{216, 0x301A}, "Earth Handstand"},
                {{217, 0x3001}, "Life Toucan"},
                {{217, 0x3005}, "Life Torch"},
                {{217, 0x300A}, "Life Hammer"},
                {{217, 0x3010}, "Life Snake"},
                {{217, 0x3018}, "Life Sword"},
                {{217, 0x301B}, "Life Yawn"},
                {{218, 0x3014}, "Dark Spider"},
                {{218, 0x3018}, "Dark Sword"},
                {{218, 0x301A}, "Dark Handstand"},
                {{219, 0x300F}, "Light Owl"},
                {{219, 0x3015}, "Light Rocket"},
                {{219, 0x301B}, "Light Yawn"},
                {{220, 0x301E}, "Kaos"},
                {{220, 0x351F}, "Kaos (Ultimate)"},
                {{230, 0x3000}, "Hand of Fate"},
                {{230, 0x3403}, "Hand of Fate (Legendary)"},
                {{231, 0x3000}, "Piggy Bank"},
                {{232, 0x3000}, "Rocket Ram"},
                {{233, 0x3000}, "Tiki Speaky"},
                {{300, 0x0000}, "Dragon's Peak"},
                {{301, 0x0000}, "Empire of Ice"},
                {{302, 0x0000}, "Pirate Seas"},
                {{303, 0x0000}, "Darklight Crypt"},
                {{304, 0x0000}, "Volcanic Vault"},
                {{305, 0x3000}, "Mirror of Mystery"},
                {{306, 0x3000}, "Nightmare Express"},
                {{307, 0x3206}, "Sunscraper Spire"},
                {{308, 0x3206}, "Midnight Museum"},
                {{404, 0x0000}, "Bash (Legendary)"},
                {{416, 0x0000}, "Spyro (Legendary)"},
                {{419, 0x0000}, "Trigger Happy (Legendary)"},
                {{430, 0x0000}, "Chop Chop (Legendary)"},
                {{450, 0x3000}, "Gusto"},
                {{451, 0x3000}, "Thunderbolt"},
                {{451, 0x301D}, "Thunderbolt (Clear)"},
                {{452, 0x3000}, "Fling Kong"},
                {{453, 0x3000}, "Blades"},
                {{453, 0x3403}, "Blades (Legendary)"},
                {{454, 0x3000}, "Wallop"},
                {{455, 0x3000}, "Head Rush"},
                {{455, 0x3402}, "Head Rush (Nitro)"},
                {{456, 0x3000}, "Fist Bump"},
                {{457, 0x3000}, "Rocky Roll"},
                {{458, 0x3000}, "Wildfire"},
                {{458, 0x3402}, "Wildfire (Dark)"},
                {{459, 0x3000}, "Ka-Boom"},
                {{460, 0x3000}, "Trail Blazer"},
                {{461, 0x3000}, "Torch"},
                {{462, 0x3000}, "Snap Shot"},
                {{462, 0x3402}, "Snap Shot (Dark)"},
                {{462, 0x450F}, "Snap Shot (Virtual)"},
                {{463, 0x3000}, "Lob-Star"},
                {{463, 0x3402}, "Lob-Star (Winterfest)"},
                {{464, 0x3000}, "Flip Wreck"},
                {{465, 0x3000}, "Echo"},
                {{466, 0x3000}, "Blastermind"},
                {{467, 0x3000}, "Enigma"},
                {{468, 0x3000}, "Deja Vu"},
                {{468, 0x3403}, "Deja Vu (Legendary)"},
                {{469, 0x3000}, "Cobra Cadabra"},
                {{469, 0x3402}, "Cobra Cadabra (King)"},
                {{470, 0x3000}, "Jawbreaker"},
                {{470, 0x3403}, "Jawbreaker (Legendary)"},
                {{471, 0x3000}, "Gearshift"},
                {{472, 0x3000}, "Chopper"},
                {{473, 0x3000}, "Tread Head"},
                {{474, 0x3000}, "Bushwhack"},
                {{474, 0x3403}, "Bushwhack (Legendary)"},
                {{475, 0x3000}, "Tuff Luck"},
                {{475, 0x301D}, "Tuff Luck (Clear)"},
                {{476, 0x3000}, "Food Fight"},
                {{476, 0x3402}, "Food Fight (Dark)"},
                {{476, 0x450F}, "Food Fight (Virtual)"},
                {{477, 0x3000}, "High Five"},
                {{478, 0x3000}, "Krypt King"},
                {{478, 0x3402}, "Krypt King (Nitro)"},
                {{479, 0x3000}, "Short Cut"},
                {{479, 0x301D}, "Short Cut (Clear)"},
                {{480, 0x3000}, "Bat Spin"},
                {{481, 0x3000}, "Funny Bone"},
                {{482, 0x3000}, "Knight Light"},
                {{483, 0x3000}, "Spotlight"},
                {{484, 0x3000}, "Knight Mare"},
                {{485, 0x3000}, "Blackout"},
                {{502, 0x3000}, "Bop"},
                {{503, 0x3000}, "Spry"},
                {{504, 0x3000}, "Hijinx"},
                {{505, 0x0000}, "Terrabite (Sidekick)"},
                {{505, 0x3000}, "Terrabite"},
                {{506, 0x3000}, "Breeze"},
                {{507, 0x3000}, "Weeruptor"},
                {{507, 0x3402}, "Weeruptor (Eggsellent)"},
                {{508, 0x3000}, "Pet Vac"},
                {{508, 0x3402}, "Pet Vac (Power Punch)"},
                {{509, 0x3000}, "Small Fry"},
                {{510, 0x3000}, "Drobit"},
                {{514, 0x0000}, "Gill Runt (Sidekick)"},
                {{514, 0x3000}, "Gill Runt"},
                {{519, 0x0000}, "Trigger Snappy (Sidekick)"},
                {{519, 0x3000}, "Trigger Snappy"},
                {{526, 0x0000}, "Whisper Elf (Sidekick)"},
                {{526, 0x3000}, "Whisper Elf"},
                {{540, 0x1000}, "Barkley (Sidekick)"},
                {{540, 0x3000}, "Barkley"},
                {{540, 0x3402}, "Barkley (Gnarly)"},
                {{541, 0x1000}, "Thumpling (Sidekick)"},
                {{541, 0x3000}, "Thumpling"},
                {{542, 0x1000}, "Mini Jini (Sidekick)"},
                {{542, 0x3000}, "Mini Jini"},
                {{543, 0x1000}, "Eye-Small (Sidekick)"},
                {{543, 0x3000}, "Eye-Small"},
                {{601, 0x5000}, "King Pen"},
                {{601, 0x5402}, "King Pen (Dark)"},
                {{602, 0x5000}, "Tri-Tip"},
                {{602, 0x5403}, "Tri-Tip (Legendary)"},
                {{603, 0x5000}, "Chopscotch"},
                {{603, 0x5415}, "Chopscotch (Candy-Coated)"},
                {{604, 0x5000}, "Boom Bloom"},
                {{605, 0x5000}, "Pit Boss"},
                {{605, 0x5403}, "Pit Boss (Legendary)"},
                {{606, 0x5000}, "Barbella"},
                {{606, 0x5021}, "Barbella (Pink)"},
                {{607, 0x5000}, "Air Strike"},
                {{607, 0x540D}, "Air Strike (Egg Bomber)"},
                {{608, 0x5000}, "Ember"},
                {{608, 0x5004}, "Ember (Dec-Ember)"},
                {{609, 0x5000}, "Ambush"},
                {{610, 0x5000}, "Dr. Krankcase"},
                {{611, 0x5000}, "Hood Sickle"},
                {{611, 0x5402}, "Hood Sickle (Silver Plated)"},
                {{612, 0x5000}, "Tae Kwon Crow"},
                {{612, 0x5402}, "Tae Kwon Crow (Mystical)"},
                {{613, 0x5000}, "Golden Queen"},
                {{613, 0x5402}, "Golden Queen (Dark)"},
                {{614, 0x5000}, "Wolfgang"},
                {{614, 0x5402}, "Wolfgang (Dark)"},
                {{615, 0x5000}, "Pain-Yatta"},
                {{615, 0x5021}, "Pain-Yatta (Rock-Candy)"},
                {{616, 0x5000}, "Mysticat"},
                {{617, 0x5000}, "Starcast"},
                {{617, 0x5021}, "Starcast (Clear/Happy Birthday)"},
                {{618, 0x5000}, "Buckshot"},
                {{618, 0x540C}, "Buckshot (Heartbreaker)"},
                {{619, 0x5000}, "Aurora"},
                {{619, 0x5402}, "Aurora (Solar Flare)"},
                {{620, 0x5000}, "Flare Wolf"},
                {{620, 0x540D}, "Flare Wolf (Hard-Boiled)"},
                {{621, 0x5000}, "Chompy Mage"},
                {{621, 0x540E}, "Chompy Mage (Jingle Bell)"},
                {{622, 0x5000}, "Bad Juju"},
                {{622, 0x5402}, "Bad Juju (Mystical)"},
                {{623, 0x5000}, "Grave Clobber"},
                {{624, 0x5000}, "Blaster-Tron"},
                {{625, 0x5000}, "Ro-Bow"},
                {{626, 0x5000}, "Chain Reaction"},
                {{626, 0x5021}, "Chain Reaction (Orange)"},
                {{627, 0x5000}, "Kaos"},
                {{628, 0x5000}, "Wild Storm"},
                {{629, 0x5000}, "Tidepool"},
                {{630, 0x5000}, "Crash Bandicoot"},
                {{631, 0x5000}, "Dr. Neo Cortex"},
                {{661, 0x5004}, "Create Your Own Skylander"},
                {{680, 0x5204}, "Magic Creation Crystal (Pyramid)"},
                {{680, 0x5208}, "Magic Creation Crystal (Lantern)"},
                {{680, 0x521B}, "Magic Creation Crystal (Claw)"},
                {{680, 0x5608}, "Magic Creation Crystal (Legendary Lantern)"},
                {{681, 0x5214}, "Water Creation Crystal (Armor)"},
                {{681, 0x5218}, "Water Creation Crystal (Fanged)"},
                {{681, 0x521C}, "Water Creation Crystal (Rocket)"},
                {{682, 0x5202}, "Air Creation Crystal (Angel)"},
                {{682, 0x5207}, "Air Creation Crystal (Lantern)"},
                {{682, 0x5212}, "Air Creation Crystal (Acorn)"},
                {{683, 0x5209}, "Undead Creation Crystal (Lantern)"},
                {{683, 0x5217}, "Undead Creation Crystal (Fanged)"},
                {{683, 0x5219}, "Undead Creation Crystal (Claw)"},
                {{684, 0x5205}, "Tech Creation Crystal (Pyramid)"},
                {{684, 0x520D}, "Tech Creation Crystal (Reactor)"},
                {{684, 0x5215}, "Tech Creation Crystal (Armor)"},
                {{685, 0x5203}, "Fire Creation Crystal (Angel)"},
                {{685, 0x520F}, "Fire Creation Crystal (Reactor)"},
                {{685, 0x5211}, "Fire Creation Crystal (Acorn)"},
                {{686, 0x520C}, "Earth Creation Crystal (Rune)"},
                {{686, 0x5213}, "Earth Creation Crystal (Armor)"},
                {{686, 0x521D}, "Earth Creation Crystal (Rocket)"},
                {{687, 0x5210}, "Life Creation Crystal (Acorn)"},
                {{687, 0x521A}, "Life Creation Crystal (Claw)"},
                {{687, 0x521E}, "Life Creation Crystal (Rocket)"},
                {{687, 0x521F}, "Life Creation Crystal (Rune)"},
                {{687, 0x5610}, "Life Creation Crystal (Legendary Acorn)"},
                {{688, 0x5206}, "Dark Creation Crystal (Pyramid)"},
                {{688, 0x520A}, "Dark Creation Crystal (Rune)"},
                {{688, 0x520E}, "Dark Creation Crystal (Reactor)"},
                {{689, 0x5201}, "Light Creation Crystal (Angel)"},
                {{689, 0x520B}, "Light Creation Crystal (Rune)"},
                {{689, 0x5216}, "Light Creation Crystal (Fanged)"},
                {{689, 0x5616}, "Light Creation Crystal (Legendary Fanged)"},
                {{699, 0x5000}, "Goldie"},
                {{1000, 0x2000}, "Boom Jet (Bottom)"},
                {{1001, 0x2000}, "Free Ranger (Bottom)"},
                {{1001, 0x2403}, "Free Ranger (Legendary) (Bottom)"},
                {{1002, 0x2000}, "Rubble Rouser (Bottom)"},
                {{1003, 0x2000}, "Doom Stone (Bottom)"},
                {{1003, 0x2016}, "Doom Stone (Gold & Bronze) (Bottom)"},
                {{1004, 0x2000}, "Blast Zone (Bottom)"},
                {{1004, 0x2402}, "Blast Zone (Dark) (Bottom)"},
                {{1005, 0x2000}, "Fire Kraken (Bottom)"},
                {{1005, 0x2004}, "Fire Kraken (Gold) (Bottom)"},
                {{1005, 0x2402}, "Fire Kraken (Jade) (Bottom)"},
                {{1006, 0x2000}, "Stink Bomb (Bottom)"},
                {{1006, 0x2016}, "Stink Bomb (Silver & Gold) (Bottom)"},
                {{1007, 0x2000}, "Grilla Drilla (Bottom)"},
                {{1008, 0x2000}, "Hoot Loop (Bottom)"},
                {{1008, 0x2402}, "Hoot Loop (Enchanted) (Bottom)"},
                {{1009, 0x2000}, "Trap Shadow (Bottom)"},
                {{1009, 0x2016}, "Trap Shadow (Bronze & Silver) (Bottom)"},
                {{1010, 0x2000}, "Magna Charge (Bottom)"},
                {{1010, 0x2402}, "Magna Charge (Nitro) (Bottom)"},
                {{1011, 0x2000}, "Spy Rise (Bottom)"},
                {{1012, 0x2000}, "Night Shift (Bottom)"},
                {{1012, 0x2403}, "Night Shift (Legendary) (Bottom)"},
                {{1013, 0x2000}, "Rattle Shake (Bottom)"},
                {{1013, 0x2402}, "Rattle Shake (Quickdraw) (Bottom)"},
                {{1014, 0x2000}, "Freeze Blade (Bottom)"},
                {{1014, 0x2402}, "Freeze Blade (Nitro) (Bottom)"},
                {{1015, 0x2000}, "Wash Buckler (Bottom)"},
                {{1015, 0x2018}, "Wash Buckler (Gold) (Bottom)"},
                {{1015, 0x2402}, "Wash Buckler (Dark) (Bottom)"},
                {{2000, 0x2000}, "Boom Jet (Top)"},
                {{2001, 0x2000}, "Free Ranger (Top)"},
                {{2001, 0x2403}, "Free Ranger (Legendary) (Top)"},
                {{2002, 0x2000}, "Rubble Rouser (Top)"},
                {{2003, 0x2000}, "Doom Stone (Top)"},
                {{2003, 0x2016}, "Doom Stone (Gold & Bronze) (Top)"},
                {{2004, 0x2000}, "Blast Zone (Top)"},
                {{2004, 0x2402}, "Blast Zone (Dark) (Top)"},
                {{2005, 0x2000}, "Fire Kraken (Top)"},
                {{2005, 0x2004}, "Fire Kraken (Gold) (Top)"},
                {{2005, 0x2402}, "Fire Kraken (Jade) (Top)"},
                {{2006, 0x2000}, "Stink Bomb (Top)"},
                {{2006, 0x2016}, "Stink Bomb (Silver & Gold) (Top)"},
                {{2007, 0x2000}, "Grilla Drilla (Top)"},
                {{2008, 0x2000}, "Hoot Loop (Top)"},
                {{2008, 0x2402}, "Hoot Loop (Enchanted) (Top)"},
                {{2009, 0x2000}, "Trap Shadow (Top)"},
                {{2009, 0x2016}, "Trap Shadow (Bronze & Silver) (Top)"},
                {{2010, 0x2000}, "Magna Charge (Top)"},
                {{2010, 0x2402}, "Magna Charge (Nitro) (Top)"},
                {{2011, 0x2000}, "Spy Rise (Top)"},
                {{2012, 0x2000}, "Night Shift (Top)"},
                {{2012, 0x2403}, "Night Shift (Legendary) (Top)"},
                {{2013, 0x2000}, "Rattle Shake (Top)"},
                {{2013, 0x2402}, "Rattle Shake (Quickdraw) (Top)"},
                {{2014, 0x2000}, "Freeze Blade (Top)"},
                {{2014, 0x2402}, "Freeze Blade (Nitro) (Top)"},
                {{2015, 0x2000}, "Wash Buckler (Top)"},
                {{2015, 0x2018}, "Wash Buckler (Gold) (Top)"},
                {{2015, 0x2402}, "Wash Buckler (Dark) (Top)"},
                {{3000, 0x2000}, "Scratch"},
                {{3001, 0x2000}, "Pop Thorn"},
                {{3002, 0x2000}, "Slobber Tooth"},
                {{3002, 0x2402}, "Slobber Tooth (Dark)"},
                {{3003, 0x2000}, "Scorp"},
                {{3003, 0x2016}, "Scorp (Green)"},
                {{3004, 0x2000}, "Fryno"},
                {{3004, 0x3801}, "Fryno (Hog Wild)"},
                {{3005, 0x2000}, "Smolderdash"},
                {{3005, 0x2206}, "Smolderdash (LightCore)"},
                {{3005, 0x2219}, "Snowderdash (LightCore)"},
                {{3006, 0x2000}, "Bumble Blast"},
                {{3006, 0x2206}, "Bumble Blast (LightCore)"},
                {{3006, 0x2402}, "Bumble Blast (Jolly)"},
                {{3007, 0x2000}, "Zoo Lou"},
                {{3007, 0x2403}, "Zoo Lou (Legendary)"},
                {{3008, 0x2000}, "Dune Bug"},
                {{3008, 0x2004}, "Dune Bug (Metallic Red)"},
                {{3009, 0x2000}, "Star Strike"},
                {{3009, 0x2206}, "Star Strike (LightCore)"},
                {{3009, 0x2602}, "Star Strike (Enchanted LightCore)"},
                {{3010, 0x2000}, "Countdown"},
                {{3010, 0x2206}, "Countdown (LightCore)"},
                {{3010, 0x2402}, "Countdown (Kickoff)"},
                {{3011, 0x2000}, "Wind Up"},
                {{3011, 0x2404}, "Wind Up (Gear Head Vicarious Visions)"},
                {{3012, 0x2000}, "Roller Brawl"},
                {{3013, 0x2000}, "Grim Creeper"},
                {{3013, 0x2206}, "Grim Creeper (LightCore)"},
                {{3013, 0x2603}, "Grim Creeper (Legendary LightCore)"},
                {{3014, 0x2000}, "Rip Tide"},
                {{3014, 0x2016}, "Rip Tide (Green)"},
                {{3015, 0x2000}, "Punk Shock"},
                {{3200, 0x2000}, "Battle Hammer"},
                {{3201, 0x2000}, "Sky Diamond"},
                {{3202, 0x2000}, "Platinum Sheep"},
                {{3203, 0x2000}, "Groove Machine"},
                {{3204, 0x2000}, "UFO Hat"},
                {{3220, 0x4000}, "Jet Stream"},
                {{3221, 0x4000}, "Tomb Buggy"},
                {{3221, 0x401E}, "Tomb Buggy (Bronze)"},
                {{3222, 0x4000}, "Reef Ripper"},
                {{3223, 0x4000}, "Burn-Cycle"},
                {{3223, 0x401E}, "Burn-Cycle (Patina)"},
                {{3224, 0x4000}, "Hot Streak"},
                {{3224, 0x4004}, "Hot Streak (Event Exclusive)"},
                {{3224, 0x411F}, "Hot Streak (Mobile)"},
                {{3224, 0x4402}, "Hot Streak (Dark)"},
                {{3224, 0x441E}, "Hot Streak (Golden)"},
                {{3224, 0x450F}, "Hot Streak (Instant)"},
                {{3225, 0x4000}, "Shark Tank"},
                {{3226, 0x4000}, "Thump Truck"},
                {{3227, 0x4000}, "Crypt Crusher"},
                {{3228, 0x4000}, "Stealth Stinger"},
                {{3228, 0x4402}, "Stealth Stinger (Nitro)"},
                {{3228, 0x450F}, "Stealth Stinger (Instant)"},
                {{3231, 0x4000}, "Dive Bomber"},
                {{3231, 0x4402}, "Dive Bomber (Spring Ahead)"},
                {{3231, 0x450F}, "Dive Bomber (Instant)"},
                {{3232, 0x4000}, "Sky Slicer"},
                {{3233, 0x4000}, "Clown Cruiser"},
                {{3233, 0x4402}, "Clown Cruiser (Dark)"},
                {{3234, 0x4000}, "Gold Rusher"},
                {{3234, 0x4402}, "Gold Rusher (Power Blue)"},
                {{3235, 0x4000}, "Shield Striker"},
                {{3235, 0x401E}, "Shield Striker (Patina)"},
                {{3236, 0x4000}, "Sun Runner"},
                {{3236, 0x4403}, "Sun Runner (Legendary)"},
                {{3237, 0x4000}, "Sea Shadow"},
                {{3237, 0x4402}, "Sea Shadow (Dark)"},
                {{3238, 0x4000}, "Splatter Splasher"},
                {{3238, 0x4402}, "Splatter Splasher (Power Blue)"},
                {{3239, 0x4000}, "Soda Skimmer"},
                {{3239, 0x4402}, "Soda Skimmer (Nitro)"},
                {{3240, 0x4000}, "Barrel Blaster"},
                {{3240, 0x4402}, "Barrel Blaster (Dark)"},
                {{3241, 0x4000}, "Buzz Wing"},
                {{3300, 0x2000}, "Sheep Wreck Island"},
                {{3301, 0x2000}, "Tower of Time"},
                {{3302, 0x2206}, "Fiery Forge"},
                {{3303, 0x2206}, "Arkeyan Crossbow"},
                {{3400, 0x4100}, "Fiesta"},
                {{3400, 0x4515}, "Fiesta (Frightful)"},
                {{3401, 0x4100}, "High Volt"},
                {{3401, 0x411E}, "High Volt (Patina)"},
                {{3402, 0x4100}, "Splat"},
                {{3402, 0x4502}, "Splat (Power Blue)"},
                {{3406, 0x4100}, "Stormblade"},
                {{3406, 0x410E}, "Stormblade (Snow-Brite)"},
                {{3411, 0x4100}, "Smash Hit"},
                {{3411, 0x4502}, "Smash Hit (Steel Plated)"},
                {{3412, 0x4100}, "Spitfire"},
                {{3412, 0x4502}, "Spitfire (Dark)"},
                {{3412, 0x450F}, "Spitfire (Instant)"},
                {{3413, 0x4100}, "Jet-Vac (Hurricane)"},
                {{3413, 0x4503}, "Jet-Vac (Legendary Hurricane)"},
                {{3414, 0x4100}, "Trigger Happy (Double Dare)"},
                {{3414, 0x4502}, "Trigger Happy (Power Blue Double Dare)"},
                {{3415, 0x4100}, "Stealth Elf (Super Shot)"},
                {{3415, 0x4502}, "Stealth Elf (Dark Super Shot)"},
                {{3415, 0x450F}, "Stealth Elf (Instant Super Shot)"},
                {{3416, 0x4100}, "Terrafin (Shark Shooter)"},
                {{3417, 0x4100}, "Roller Brawl (Bone Bash)"},
                {{3417, 0x411E}, "Roller Brawl (Bronze Bone Bash)"},
                {{3417, 0x4503}, "Roller Brawl (Legendary Bone Bash)"},
                {{3420, 0x4100}, "Pop Fizz (Big Bubble)"},
                {{3420, 0x450E}, "Pop Fizz (Birthday Bash Big Bubble)"},
                {{3421, 0x4100}, "Eruptor (Lava Lance)"},
                {{3421, 0x411E}, "Eruptor (Patina Lava Lance)"},
                {{3422, 0x4100}, "Gill Grunt (Deep Dive)"},
                {{3423, 0x4100}, "Donkey Kong (Turbo Charge)"},
                {{3423, 0x4502}, "Donkey Kong (Dark Turbo Charge)"},
                {{3424, 0x4100}, "Bowser (Hammer Slam)"},
                {{3424, 0x4502}, "Bowser (Dark Hammer Slam)"},
                {{3425, 0x4100}, "Dive-Clops"},
                {{3425, 0x450E}, "Dive-Clops (Missile-Tow)"},
                {{3425, 0x450F}, "Dive-Clops (Instant)"},
                {{3426, 0x4100}, "Astroblast"},
                {{3426, 0x4503}, "Astroblast (Legendary)"},
                {{3427, 0x4100}, "Nightfall"},
                {{3428, 0x4100}, "Thrillipede"},
                {{3428, 0x450D}, "Thrillipede (Eggcited)"},
                {{3500, 0x4000}, "Sky Trophy"},
                {{3501, 0x4000}, "Land Trophy"},
                {{3502, 0x4000}, "Sea Trophy"},
                {{3503, 0x4000}, "Kaos Trophy"},
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
    }
    return "None";
}

void SkylanderPortal::Skylander::Save() {
    if (!skyFile)
        return;

    fseeko(skyFile, 0, SEEK_SET);
    fwrite(data.data(), data.size(), sizeof(data[0]), skyFile);
}
