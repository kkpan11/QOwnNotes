#!/usr/bin/env bash

set -euo pipefail

project=${OBS_PROJECT:-home:pbek:QOwnNotes}
package=${OBS_TARGET_PACKAGE:-desktop-cmake3}

# Keep this list limited to distributions that provide the Qt6 development
# packages required by the CMake, Debian, RPM, or Arch build descriptions.
repositories=(
  16.0
  Arch
  Arch_Extra
  Debian_12
  Debian_13
  Fedora_36
  Fedora_37
  Fedora_38
  Fedora_39
  Fedora_40
  Fedora_41
  Fedora_42
  Fedora_43
  Fedora_44
  Fedora_Rawhide
  Raspbian_12
  Raspbian_13
  openSUSE_Tumbleweed
  xUbuntu_23.04
  xUbuntu_23.10
  xUbuntu_24.04
  xUbuntu_24.10
  xUbuntu_25.04
  xUbuntu_25.10
  xUbuntu_26.04
)

command -v osc >/dev/null || {
  echo "osc is required to update the OBS package metadata." >&2
  exit 1
}

metadata=$(mktemp)
trap 'rm -f "$metadata"' EXIT

osc api "/source/$project/$package/_meta" >"$metadata"

python3 - "$metadata" "${repositories[@]}" <<'PY'
import sys
import xml.etree.ElementTree as ET

path, *repositories = sys.argv[1:]
tree = ET.parse(path)
root = tree.getroot()

for section_name in ("build", "publish", "debuginfo", "useforbuild"):
    section = root.find(section_name)
    if section is None:
        section = ET.SubElement(root, section_name)
    section.clear()
    ET.SubElement(section, "disable")
    for repository in repositories:
        ET.SubElement(section, "enable", repository=repository)

ET.indent(tree, space="  ")
tree.write(path, encoding="unicode")
PY

osc api -X PUT -T "$metadata" "/source/$project/$package/_meta"

echo "Enabled Qt6-capable repositories for $project/$package."
