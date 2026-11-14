#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_BINARY="$(mktemp /tmp/nima-reaching-test.XXXXXX)"
trap 'rm -f -- "$TEST_BINARY"' EXIT
g++ -std=c++17 -O2 -Wall -Wextra -Werror "$PROJECT_DIR/core/tests/test_nima_reaching.cpp" -o "$TEST_BINARY"
"$TEST_BINARY"
python3 - "$PROJECT_DIR" <<'PY'
import sys
from pathlib import Path
root=Path(sys.argv[1])
sys.path.insert(0,str(root/'python_client'))
from parser import xml_parser
config=xml_parser(str(root/'python_client/config/nima_reaching.xml'))
assert config.end_conditions.conditions[0].name=='nima_reaching'
assert len(config.objects)==2 and not config.test
print('PASS: existing Haptech XML parser accepts nima_reaching.xml')
PY
