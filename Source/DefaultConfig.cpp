#include "DefaultConfig.h"

std::string g_defaultConfigText =
R"term(#This is the configuration file for MinerManager
#Below are the default settings created when MinerManager.config is not found

$INFORMATION$
Version = 0.1

$SETTINGS$
UpdateRate = 5.0 #Seconds
StartProcessesMinimized = true #This will start the miner process(es) minimized
ExecutableName = PhoenixMiner.exe #Either the full path or the local path to the miner executable
AfterburnerLocation = C:\Program Files (x86)\MSI Afterburner\MSIAfterburner.exe

#Please list processes you would want the miner to stop for
#Inclusives will stop the mining executible if the phrase is       found in      the process name and is case insensitive
#Exlusives  will stop the mining executible if the phrase is an exact match with the process name and is case sensitive

$INCLUSIVE$
F1_20 #F1_2019_dx12.exe and F1_2012.exe
doom #DOOMx64vk.exe and DOOMEternalx64vk.exe
civilization #CivilizationV_DX11.exe and CivilizationVI_DX12.exe
EscapeFromTarkov #EscapeFromTarkov_BE.exe
halo

$EXCLUSIVE$
factorio.exe
League of Legends.exe
Brawler.exe
vermintide3_dx12.exe
Smite.exe
Cyberpunk.exe
BlackOpsColdWar.exe
Valorant.exe
NMS.exe
Borderlands3.exe
RDR2.exe
bfv.exe
OrcsMustDie2.exe
ItTakesTwo.exe
wow.exe
PortalWars-Win64-Shipping.exe
myst.exe
Kena-Win64-Shipping.exe
csgo.exe
bf.exe
DyingLightGame.exe
Warcraft.exe
Wow.exe
WowClassic.exe
SC2_x64.exe

ktane.exe
TOXIKK.exe
One Finger Death Punch.exe
battlevschess.exe
castle.exe
dirt3_game.exe
Disco Dodgeball.exe
dontstarve_steam.exe
DungeonSouls.exe
eurotrucks2.exe
final_exam.exe
FullMojo.exe
Guac.exe
Hammerwatch.exe
Hero_Siege.exe
kofxiii.exe
WizardWarsLauncher.exe
Not the Robots.exe
PathOfExileSteam.exe
quakelive_steam.exe
sir.exe
SpeedRunners.exe
starbound.exe
starbound_opengl.exe
Tabletop Simulator.exe
toribash.exe
Unturned.exe
aces.exe
arma2.exe
ArmA2OA.exe
arma3.exe
maniaplanet.exe
broforce_beta.exe
HeroesAndGeneralsDesktop.exe
hng.exe
downwell.exe
RebelGalaxy.exe
RebelGalaxyGOG.exe
vermintide.exe
Star_Trek_Online.exe
Scrivener.exe
dfbhd.exe
Atilla.exe
Bastion.exe
BloodBowl2.exe
BloodBowl2_DX_32.exe
BloodBowl2_GL_32.exe
Borderlands.exe
Borderlands2.exe
Brothers.exe
Cities.exe
Darkest.exe
Darksiders2.exe
defcon.exe
DMC-DevilMayCry.exe
dontstarve.exe
DotP_D14.exe
DragonAgeInquisition.exe
DungeonoftheEndless.exe
DustAET.exe
Dwarf Fortress.exe
DXHRDC.exe
ed6_win.exe
EndlessLegend.exe
eu4.exe
FTLGame.exe
GalCiv2.exe
GeometryWars.exe
Hexcells Plus.exe
Hexcells.exe
HotlineGL.exe
HotlineMiami.exe
Kingdom.exe
KSP.exe
MetroLL.exe
MKKE.exe
nuclearthrone.exe
PAC-MAN.exe
payday2_win32_release.exe
PlanetSide2_x64.exe
Polynomial.exe
Prison Architect.exe
ProjectZomboid.exe
Really Big Sky.exe
ReassemblyRelease.exe
RebelGalaxyGOG.exe
RebelGalaxySteam.exe
reprisaluniverse.exe
Reus.exe
Rome2.exe
SanctumGame-Win32-Shipping.exe
Scribble.exe
Se4.exe
sh4.exe
shatteredplanet.exe
Shogun2.exe
ShooterGame.exe
Sins of a Solar Empire Rebellion.exe
SkullGirls.exe
sots2.exe
SpaceChem.exe
StarDrive.exe
starwarsbattlefront.exe
Sunless Sea.exe
superhexagon.exe
SuperMeatBoy.exe
Terraria.exe
They Bleed Pixels PC.exe
Timberman.exe
TombRaider.exe
Torchlight2.exe
Trine.exe
Trine2_32bit.exe
UNDERTALE.exe
uplink.exe
v2game.exe
Waves.exe
Wildstar.exe
Windward.exe
WL2.exe
WorldOfGoo.exe
X3AP.exe
X3AP_n.exe
X3TC.exe
XRebirth.exe
yso_win.exe
Zigfrak.exe
Ziggurat.exe
TmUnitedForever.exe
EDLaunch.exe
THUG.exe
UE4-Win64-Test.exe
Tales of Zestiria.exe
FlagAC4BFSP.exe
AIWar.exe
Application-x64.exe
Audiosurf2.exe
Gnomoria.exe
NEOScavenger.exe
oolite.exe
Overture.exe
Pandora.exe
sspace.exe
SublevelZero.exe
Tidalis.exe
X-Plane-32bit.exe
X-Plane.exe
WAR.exe
RobloxPlayerBeta.exe
RobloxStudioBeta.exe

)term";

