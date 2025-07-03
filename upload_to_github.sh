#!/bin/bash

# GitHub Upload Script for AMCheck C++
# Run this script from the cpp directory: f:\VSCODE\amcheck\cpp

set -e

echo "=========================================="
echo "   AMCheck C++ GitHub Upload Script"
echo "=========================================="

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]] || [[ ! -f "README.md" ]]; then
    echo "❌ Error: Please run this script from the cpp directory containing CMakeLists.txt"
    exit 1
fi

echo "✅ Verified: In correct project directory"

# Check if git is initialized
if [[ ! -d ".git" ]]; then
    echo "📁 Initializing git repository..."
    git init
else
    echo "✅ Git repository already initialized"
fi

# Configure git user (update with your details)
echo "⚙️  Configuring git user..."
git config user.name "Nasir Ali"
git config user.email "nasiraliphy@gmail.com"

# Check if .gitignore exists
if [[ ! -f ".gitignore" ]]; then
    echo "❌ Error: .gitignore file not found. Please create it first."
    exit 1
fi

echo "✅ .gitignore file found"

# Clean any build artifacts before committing
echo "🧹 Cleaning build artifacts..."
#if [[ -d "build" ]]; then
#    echo "Removing build directory..."
#    rm -rf build
#fi

# Add all files
echo "📁 Adding files to git..."
git add .

# Check if there are changes to commit
if git diff --staged --quiet; then
    echo "ℹ️  No changes to commit"
else
    echo "📝 Committing changes..."
    git commit -m "Initial commit: AMCheck C++ - High-Performance Altermagnet Detection Tool

Features:
- Complete C++ implementation with Eigen3 and spglib integration
- Cross-platform support (Linux, Windows MSYS2, macOS)
- Multithreaded comprehensive spin configuration search
- Standalone executable generation with static linking
- Interactive and batch processing modes
- VASP POSCAR file support with robust parsing
- Anomalous Hall Coefficient analysis
- Automated build scripts and verification tools
- Comprehensive documentation and troubleshooting guides

Authors: Nasir Ali, Shah Faisal
Supervisor: Prof. Dr. Gul Rahman
Institution: Department of Physics, Quaid-i-Azam University, Pakistan"
fi

# Check if remote origin exists
if git remote get-url origin 2>/dev/null; then
    echo "✅ Remote origin already configured"
else
    echo "🔗 Adding GitHub repository as remote origin..."
    git remote add origin https://github.com/nasirxo/amcheckcpp.git
fi

# Show remote configuration
echo "📡 Remote configuration:"
git remote -v

# Check current branch
CURRENT_BRANCH=$(git branch --show-current)
echo "🌿 Current branch: $CURRENT_BRANCH"

# Rename to main if necessary
if [[ "$CURRENT_BRANCH" != "main" ]]; then
    echo "🔄 Renaming branch to 'main'..."
    git branch -M main
fi

# Push to GitHub
echo "🚀 Pushing to GitHub..."
echo ""
echo "⚠️  Note: You may be prompted for authentication."
echo "   If using Personal Access Token:"
echo "   - Username: nasirxo"
echo "   - Password: [Your Personal Access Token]"
echo ""
echo "   Generate token at: https://github.com/settings/tokens"
echo "   Required scopes: repo, workflow"
echo ""

if git push -u origin main; then
    echo ""
    echo "🎉 SUCCESS! Project uploaded to GitHub!"
    echo ""
    echo "📍 Repository URL: https://github.com/nasirxo/amcheckcpp"
    echo "🌐 You can now visit your repository in a web browser"
    echo ""
    echo "📋 Next steps:"
    echo "1. Visit the repository to verify all files are uploaded"
    echo "2. Update repository description and topics on GitHub"
    echo "3. Create releases for stable versions"
    echo "4. Set up GitHub Actions for automated building (optional)"
    echo ""
else
    echo ""
    echo "❌ Failed to push to GitHub"
    echo ""
    echo "🔧 Troubleshooting:"
    echo "1. Check your internet connection"
    echo "2. Verify GitHub credentials (username/token)"
    echo "3. Ensure repository exists and you have write access"
    echo "4. Try using SSH authentication instead"
    echo ""
    echo "💡 Alternative: Use GitHub Desktop or upload via web interface"
    exit 1
fi

echo "=========================================="
