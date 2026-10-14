#!/bin/sh
# publish_public.sh - pushes local/play to the public repo (github.com/iloupv/leon-ops-zombies-kennedy, main) without
# the files that are not the project's to publish, removed from every commit of its history:
#   - Valve's Steam client DLLs the decomp shipped (the exe needs only steam_api.dll, the SDK's redistributable)
#   - full copies of Treyarch's maps/_zombiemode_ffotd.gsc (the decomp's mods/_stack and mods/zinfo examples)
#   - mods/_stack's .ents files (entity placements taken from the retail maps)
# The local repo keeps them (those example mods use them). git filter-branch is deterministic - the same commits
# filter to the same commits - so each run's history extends the last one's and the push is a fast-forward.
#
#   sh tools/leon-publish/publish_public.sh            (from the repo root, Git Bash)
#   sh tools/leon-publish/publish_public.sh --force    (only to replace a history that was published unfiltered)
set -e
REPO=$(cd "$(dirname "$0")/../.." && pwd)
PUBLIC=https://github.com/iloupv/leon-ops-zombies-kennedy.git
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
git clone --quiet --no-local --single-branch --branch local/play "$REPO" "$WORK/public"
cd "$WORK/public"
FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch --prune-empty --index-filter '
  git rm -q --cached --ignore-unmatch \
    src/steam/redistributable_bin/steamclient.dll src/steam/redistributable_bin/tier0_s.dll \
    src/steam/redistributable_bin/vstdlib_s.dll src/steam/redistributable_bin/Steam.dll \
    mods/_stack/maps/_zombiemode_ffotd.gsc mods/zinfo/maps/_zombiemode_ffotd.gsc "mods/_stack/maps/*.ents"
' -- local/play
for f in src/steam/redistributable_bin/steamclient.dll mods/_stack/maps/_zombiemode_ffotd.gsc \
         mods/zinfo/maps/_zombiemode_ffotd.gsc mods/_stack/maps/zombie_theater.zombie_moon.ents; do
  if [ -n "$(git log --oneline -1 -- "$f")" ]; then echo "still in the history: $f" >&2; exit 1; fi
done
if [ "$1" = "--force" ]; then
  git push --force "$PUBLIC" local/play:main
else
  git push "$PUBLIC" local/play:main
fi
git rev-parse local/play
