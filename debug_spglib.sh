#!/bin/bash

echo "🔍 Debugging spglib installation..."
echo "========================================"

# Check package installation
echo "📦 Checking installed packages:"
if command -v dpkg &> /dev/null; then
    dpkg -l | grep -i spg || echo "No spglib packages found via dpkg"
elif command -v rpm &> /dev/null; then
    rpm -qa | grep -i spg || echo "No spglib packages found via rpm"
fi

echo ""
echo "📋 Checking pkg-config:"
if command -v pkg-config &> /dev/null; then
    echo "  symspg: $(pkg-config --exists symspg && echo '✅ Found' || echo '❌ Not found')"
    echo "  spglib: $(pkg-config --exists spglib && echo '✅ Found' || echo '❌ Not found')"
    
    if pkg-config --exists symspg; then
        echo "  symspg version: $(pkg-config --modversion symspg)"
        echo "  symspg cflags: $(pkg-config --cflags symspg)"
        echo "  symspg libs: $(pkg-config --libs symspg)"
    fi
else
    echo "  pkg-config not found"
fi

echo ""
echo "📁 Searching for spglib.h:"
find /usr/include -name "spglib.h" 2>/dev/null | head -5 || echo "  Not found in /usr/include"

echo ""
echo "📚 Searching for spglib libraries:"
echo "  libsymspg*:"
find /usr/lib* -name "libsymspg*" 2>/dev/null | head -5 || echo "    Not found"
echo "  libspg*:"
find /usr/lib* -name "libspg*" 2>/dev/null | head -5 || echo "    Not found"

echo ""
echo "🧪 Testing C compilation:"
cat > /tmp/test_spglib.c << 'EOF'
#include <stdio.h>
#include <spglib.h>

int main() {
    printf("spglib version: %d.%d.%d\n", 
           spg_get_major_version(), 
           spg_get_minor_version(), 
           spg_get_micro_version());
    return 0;
}
EOF

if gcc /tmp/test_spglib.c -lsymspg -o /tmp/test_spglib 2>/dev/null; then
    echo "✅ Compilation successful with -lsymspg"
    /tmp/test_spglib 2>/dev/null || echo "⚠️ Execution failed"
elif gcc /tmp/test_spglib.c -lspg -o /tmp/test_spglib 2>/dev/null; then
    echo "✅ Compilation successful with -lspg"
    /tmp/test_spglib 2>/dev/null || echo "⚠️ Execution failed"
else
    echo "❌ Compilation failed"
    echo "Try installing with: sudo apt-get install libsymspg-dev"
fi

# Cleanup
rm -f /tmp/test_spglib.c /tmp/test_spglib

echo ""
echo "🎯 Recommendation:"
if pkg-config --exists symspg || find /usr/lib* -name "libsymspg*" 2>/dev/null | grep -q .; then
    echo "✅ spglib appears to be installed correctly"
    echo "   Try rebuilding with: ./clean.sh && ./build.sh"
else
    echo "❌ spglib not found. Install with:"
    echo "   sudo apt-get install libsymspg-dev"
fi
