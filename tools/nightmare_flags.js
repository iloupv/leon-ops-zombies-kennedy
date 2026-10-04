// The nightmare mod's + commands (mods/nightmare/README.md), in ONE place: tools/mod-launcher.hta loads this file (<script src>)
// and tools/mergecheck.sh's check_nightmare runs it with node. Plain ES3 (the HTA's JScript and node both run it).
// nightmareFlags(level, a, map): level 0 = Normal (a unchanged), 1 = Nightmare, 2 = Hell; a = the command line built so far
// (with or without tools/horde_flags.js's flags); map = the devmap name (L61: Kino adds zombie_pentagon for the thief; L62: and zombie_cosmodrome for the space monkeys). Five (zombie_pentagon) and Kino (zombie_theater, L61): spawner files
// mods/_stack/maps/<map>.zombie_<zone>.ents.
// The mod's scripts link Shangri-La's, Call of the Dead's and Moon's AI scripts, so all three zones are always loaded
// (without one of them the level's scripts fail to compile).
var NIGHTMARE_ZONES = "zombie_temple zombie_coast zombie_moon";
function nightmareFlags(level, a, map) {
  var l = parseInt(level, 10);
  if (isNaN(l) || l <= 0) return a;
  a = a || "";
  // fs_mods: nightmare after horde (horde's hooks first), before zinfo/noperks; fs_game keeps horde's when horde is on
  if (/\+set fs_mods horde /.test(a)) a = a.replace("+set fs_mods horde ", "+set fs_mods horde nightmare ");
  else if (/\+set fs_mods /.test(a)) a = a.replace("+set fs_mods ", "+set fs_mods nightmare ");
  else a += " +set fs_mods nightmare";
  if (!/\+set fs_game /.test(a)) a += " +set fs_game mods/nightmare";
  // one bo1_mod_zones (horde's napalm/shrieker set it to zombie_temple)
  a = a.replace(/ ?\+set bo1_mod_zones [a-z_ ]*?(?= \+|$)/, "");
  a += " +set bo1_mod_zones " + NIGHTMARE_ZONES + (map == "zombie_theater" ? " zombie_pentagon zombie_cosmodrome" : "") + " +set nightmare_level " + (l >= 2 ? 2 : 1);
  return a.replace(/^ /, "");
}
if (typeof module != "undefined") module.exports = nightmareFlags;
