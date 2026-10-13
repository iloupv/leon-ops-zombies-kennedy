// LEON MOD - the mod's entry point: the engine runs maps\_mod_main::main for an fs_game mod right after the level's
// main, on every map (G_SP_LoadLevelScripts), so Leon plays on any zombie map without a changed retail script.

main()
{
	maps\leon\_leon::init(); // its precaches and effects now, in the level's first frame
}
