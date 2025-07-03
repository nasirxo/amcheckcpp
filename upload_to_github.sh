#!/bin/bash

# GitHub Upload Script for AMCheck C++ (SSH Version)
# Run this script from the cpp directory: f:\VSCODE\amcheck\cpp

set -e

# Unicode symbols and emoji for beautiful output on all platforms
CHECK="✅"
CROSS="❌"
INFO="ℹ️"
ARROW="🔄"
GEAR="⚙️"
CLEAN="🧹"
FOLDER="📁"
COMMIT="📝"
REMOTE="📡"
BRANCH="🌿"
PUSH="🚀"
SUCCESS="🎉"
FAIL="❌"
KEY="🔐"
TROUBLE="🔧"

echo "╔══════════════════════════════════════════════════════════════════════════╗"
echo "║                                                                          ║"
echo "║            🚀 AMCheck C++ GitHub Upload Script 🚀                        ║"
echo "║                  (SSH Authentication)                                    ║"
echo "║                                                                          ║"
echo "╚══════════════════════════════════════════════════════════════════════════╝"

# Random commit message arrays
COMMIT_PREFIXES=(
    "Update AMCheck C++ implementation"
    "Enhance altermagnet detection features"
    "Improve crystallographic analysis tools"
    "Refine multithreaded spin configuration"
    "Optimize standalone executable generation"
    "Update cross-platform build system"
    "Enhance VASP POSCAR file support"
    "Improve documentation and examples"
    "Refactor symmetry operations handling"
    "Update anomalous Hall coefficient analysis"
)

COMMIT_DETAILS=(
    "- Enhanced performance and stability"
    "- Improved error handling and validation"
    "- Updated documentation and examples"
    "- Cross-platform compatibility improvements"
    "- Optimized memory usage and algorithms"
    "- Better multithreading implementation"
    "- Enhanced user interface and output"
    "- Improved build scripts and dependencies"
    "- Code cleanup and refactoring"
    "- Updated libraries and dependencies"
    "- Better error messages and diagnostics"
    "- Performance optimizations"
    "- Enhanced standalone binary generation"
    "- Improved spglib integration"
    "- Better symmetry analysis algorithms"
)

# Function to generate random commit message
generate_commit_message() {
    local prefix_idx=$((RANDOM % ${#COMMIT_PREFIXES[@]}))
    local detail1_idx=$((RANDOM % ${#COMMIT_DETAILS[@]}))
    local detail2_idx=$((RANDOM % ${#COMMIT_DETAILS[@]}))
    
    # Ensure different details
    while [ $detail2_idx -eq $detail1_idx ]; do
        detail2_idx=$((RANDOM % ${#COMMIT_DETAILS[@]}))
    done
    
    echo "${COMMIT_PREFIXES[$prefix_idx]}

${COMMIT_DETAILS[$detail1_idx]}
${COMMIT_DETAILS[$detail2_idx]}

Authors: Nasir Ali, Shah Faisal
Institution: Department of Physics, Quaid-i-Azam University"
}

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]] || [[ ! -f "README.md" ]]; then
    echo "$CROSS Error: Please run this script from the cpp directory containing CMakeLists.txt"
    exit 1
fi

echo "$CHECK Verified: In correct project directory"

# Check if git is initialized
if [[ ! -d ".git" ]]; then
    echo "$FOLDER Initializing git repository..."
    git init
else
    echo "$CHECK Git repository already initialized"
fi

# Configure git user (update with your details)
echo "$GEAR Configuring git user..."
git config user.name "Nasir Ali"
git config user.email "nasiraliphy@gmail.com"

# Check if .gitignore exists
if [[ ! -f ".gitignore" ]]; then
    echo "$CROSS Error: .gitignore file not found. Please create it first."
    exit 1
fi

echo "$CHECK .gitignore file found"

# Clean any build artifacts before committing
echo "$CLEAN Cleaning build artifacts..."
#if [[ -d "build" ]]; then
#    echo "Removing build directory..."
#    rm -rf build
#fi

# Add all files
echo "$FOLDER Adding files to git..."
git add .

# Check if there are changes to commit
if git diff --staged --quiet; then
    echo "$INFO No changes to commit"
else
    echo "$COMMIT Committing changes..."
    COMMIT_MSG=$(generate_commit_message)
    echo "Generated commit message:"
    echo "---"
    echo "$COMMIT_MSG"
    echo "---"
    git commit -m "$COMMIT_MSG"
fi

# Check if remote origin exists
if git remote get-url origin 2>/dev/null; then
    echo "$CHECK Remote origin already configured"
    CURRENT_REMOTE=$(git remote get-url origin)
    
    # Check if remote is using SSH
    if [[ "$CURRENT_REMOTE" == git@github.com:* ]]; then
        echo "$CHECK SSH remote already configured: $CURRENT_REMOTE"
    else
        echo "$ARROW Converting HTTPS remote to SSH..."
        git remote set-url origin git@github.com:nasirxo/amcheckcpp.git
        echo "$CHECK Remote converted to SSH"
    fi
else
    echo "$REMOTE Adding GitHub repository as SSH remote..."
    git remote add origin git@github.com:nasirxo/amcheckcpp.git
fi

# Show remote configuration
echo "$REMOTE Remote configuration:"
git remote -v

# Check current branch
CURRENT_BRANCH=$(git branch --show-current)
echo "$BRANCH Current branch: $CURRENT_BRANCH"

# Rename to main if necessary
if [[ "$CURRENT_BRANCH" != "main" ]]; then
    echo "$ARROW Renaming branch to 'main'..."
    git branch -M main
fi

# Push to GitHub
echo "$PUSH Pushing to GitHub via SSH..."
echo ""
echo "$KEY Using SSH authentication (no token required)"

# Test SSH connection first
echo "Testing SSH connection..."
if ssh -T git@github.com 2>&1 | grep -q "successfully authenticated"; then
    echo "$CHECK SSH authentication successful"
else
    echo "$CROSS SSH authentication failed"
    echo ""
    echo "$TROUBLE SSH Setup Required:"
    echo "1. Generate SSH key: ssh-keygen -t ed25519 -C 'nasiraliphy@gmail.com'"
    echo "2. Add to SSH agent: ssh-add ~/.ssh/id_ed25519"
    echo "3. Copy public key: cat ~/.ssh/id_ed25519.pub"
    echo "4. Add to GitHub: https://github.com/settings/ssh/new"
    echo "5. Test connection: ssh -T git@github.com"
    exit 1
fi

if git push -u origin main; then
    echo ""
    echo "$SUCCESS SUCCESS! Project uploaded to GitHub!"
    echo ""
    echo "Repository URL: https://github.com/nasirxo/amcheckcpp"
    echo "You can now visit your repository in a web browser"
    echo ""
    echo "Next steps:"
    echo "1. Visit the repository to verify all files are uploaded"
    echo "2. Update repository description and topics on GitHub"
    echo "3. Create releases for stable versions"
    echo "4. Set up GitHub Actions for automated building (optional)"
    echo ""
else
    echo ""
    echo "$FAIL Failed to push to GitHub via SSH"
    echo ""
    echo "$TROUBLE Troubleshooting:"
    echo "1. Check SSH key setup: ssh -T git@github.com"
    echo "2. Verify repository exists and you have write access"
    echo "3. Check internet connection"
    echo "4. Ensure SSH key is added to GitHub account"
    echo ""
    echo "$KEY SSH Key Management:"
    echo "- List keys: ssh-add -l"
    echo "- Add key: ssh-add ~/.ssh/id_ed25519"
    echo "- Generate new key: ssh-keygen -t ed25519 -C 'nasiraliphy@gmail.com'"
    echo ""
    exit 1
fi

echo "=========================================="
