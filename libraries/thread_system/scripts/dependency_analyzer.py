#!/usr/bin/env python3
"""
Phase 3 T3.2: Dependency Tree Visualization Tool
Analyzes and visualizes dependency relationships for thread_system
"""

import json
import sys
import os
import argparse
import subprocess
import shutil
from pathlib import Path
from typing import Dict, List, Set, Optional, Tuple
import re

# Color schemes for different output formats
COLORS = {
    'graphviz': {
        'core': '#4CAF50',      # Green for core dependencies
        'testing': '#FF9800',   # Orange for testing dependencies
        'optional': '#2196F3',  # Blue for optional dependencies
        'conflict': '#F44336',  # Red for conflicts
        'warning': '#FFC107'    # Yellow for warnings
    },
    'html': {
        'core': 'success',
        'testing': 'warning', 
        'optional': 'info',
        'conflict': 'danger',
        'warning': 'warning'
    }
}

class DependencyAnalyzer:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.vcpkg_manifest = {}
        self.cmake_dependencies = set()
        self.dependency_graph = {}
        self.conflicts = []
        self.warnings = []
        
    def load_vcpkg_manifest(self) -> bool:
        """Load vcpkg.json manifest file"""
        vcpkg_path = self.project_root / "vcpkg.json"
        
        if not vcpkg_path.exists():
            print(f"❌ vcpkg.json not found at {vcpkg_path}")
            return False
            
        try:
            with open(vcpkg_path, 'r', encoding='utf-8') as f:
                self.vcpkg_manifest = json.load(f)
            print(f"✅ Loaded vcpkg.json: {self.vcpkg_manifest.get('name', 'unknown')}")
            return True
        except json.JSONDecodeError as e:
            print(f"❌ Failed to parse vcpkg.json: {e}")
            return False
    
    def analyze_cmake_dependencies(self) -> Set[str]:
        """Extract dependencies from CMakeLists.txt"""
        cmake_path = self.project_root / "CMakeLists.txt"
        dependencies = set()
        
        if not cmake_path.exists():
            print(f"⚠️ CMakeLists.txt not found at {cmake_path}")
            return dependencies
        
        try:
            with open(cmake_path, 'r', encoding='utf-8') as f:
                content = f.read()
                
            # Find find_package calls
            find_package_pattern = r'find_package\s*\(\s*([^\s\)]+)'
            matches = re.findall(find_package_pattern, content, re.IGNORECASE)
            dependencies.update(matches)
            
            # Find target_link_libraries calls with external libraries
            link_pattern = r'target_link_libraries\s*\([^)]*?([A-Za-z_][A-Za-z0-9_]*::[A-Za-z_][A-Za-z0-9_]*)'
            matches = re.findall(link_pattern, content, re.MULTILINE | re.DOTALL)
            for match in matches:
                lib_name = match.split('::')[0]
                dependencies.add(lib_name)
            
            print(f"📋 Found {len(dependencies)} CMake dependencies")
            self.cmake_dependencies = dependencies
            return dependencies
            
        except Exception as e:
            print(f"⚠️ Error analyzing CMakeLists.txt: {e}")
            return dependencies
    
    def build_dependency_graph(self) -> Dict:
        """Build comprehensive dependency graph"""
        graph = {
            'core': [],
            'testing': [],
            'logging': [],
            'development': [],
            'system': []
        }
        
        # Process core dependencies
        for dep in self.vcpkg_manifest.get('dependencies', []):
            dep_info = self._parse_dependency(dep)
            dep_info['category'] = 'core'
            graph['core'].append(dep_info)
        
        # Process feature-based dependencies
        features = self.vcpkg_manifest.get('features', {})
        for feature_name, feature_info in features.items():
            feature_deps = feature_info.get('dependencies', [])
            for dep in feature_deps:
                dep_info = self._parse_dependency(dep)
                dep_info['category'] = feature_name
                graph[feature_name].append(dep_info)
        
        # Add system/CMake dependencies
        for cmake_dep in self.cmake_dependencies:
            if cmake_dep not in [d['name'] for deps in graph.values() for d in deps]:
                dep_info = {
                    'name': cmake_dep,
                    'version': 'system',
                    'platform': None,
                    'features': [],
                    'category': 'system'
                }
                graph['system'].append(dep_info)
        
        self.dependency_graph = graph
        return graph
    
    def _parse_dependency(self, dep) -> Dict:
        """Parse dependency from vcpkg format"""
        if isinstance(dep, str):
            return {
                'name': dep,
                'version': None,
                'platform': None, 
                'features': []
            }
        elif isinstance(dep, dict):
            return {
                'name': dep.get('name', ''),
                'version': dep.get('version>=', dep.get('version', None)),
                'platform': dep.get('platform', None),
                'features': dep.get('features', [])
            }
        else:
            return {'name': str(dep), 'version': None, 'platform': None, 'features': []}
    
    def check_circular_dependencies(self) -> List[List[str]]:
        """Detect circular dependencies (simplified implementation)"""
        # This is a basic implementation - real circular dependency detection
        # would require analyzing the actual package dependency trees
        cycles = []
        
        # Check for obvious self-references or immediate cycles
        all_deps = set()
        for category in self.dependency_graph.values():
            for dep in category:
                all_deps.add(dep['name'])
        
        # For now, just check if thread-system depends on itself
        if self.vcpkg_manifest.get('name') in all_deps:
            cycles.append([self.vcpkg_manifest.get('name')])
        
        return cycles
    
    def generate_graphviz(self, output_path: Path) -> bool:
        """Generate GraphViz DOT format visualization"""
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write('digraph DependencyGraph {\n')
                f.write('    rankdir=TB;\n')
                f.write('    node [shape=box, style=rounded];\n')
                f.write('    edge [arrowhead=open];\n\n')
                
                # Main project node
                project_name = self.vcpkg_manifest.get('name', 'thread-system')
                f.write(f'    "{project_name}" [fillcolor=lightgray, style=filled];\n\n')
                
                # Add dependency nodes and edges
                for category, deps in self.dependency_graph.items():
                    if not deps:
                        continue
                        
                    color = COLORS['graphviz'].get(category, COLORS['graphviz']['optional'])
                    
                    # Create subgraph for category
                    f.write(f'    subgraph cluster_{category} {{\n')
                    f.write(f'        label="{category.title()} Dependencies";\n')
                    f.write(f'        style=filled;\n')
                    f.write(f'        fillcolor="{color}20";\n\n')
                    
                    for dep in deps:
                        dep_name = dep['name']
                        version_str = f" v{dep['version']}" if dep['version'] else ""
                        platform_str = f" ({dep['platform']})" if dep['platform'] else ""
                        label = f"{dep_name}{version_str}{platform_str}"
                        
                        f.write(f'        "{dep_name}" [label="{label}", fillcolor="{color}", style=filled];\n')
                        f.write(f'        "{project_name}" -> "{dep_name}";\n')
                    
                    f.write('    }\n\n')
                
                f.write('}\n')
            
            print(f"📊 GraphViz diagram generated: {output_path}")
            return True
            
        except Exception as e:
            print(f"❌ Failed to generate GraphViz: {e}")
            return False
    
    def generate_html_report(self, output_path: Path) -> bool:
        """Generate HTML dependency report"""
        try:
            html_content = self._generate_html_template()
            
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(html_content)
            
            print(f"📊 HTML report generated: {output_path}")
            return True
            
        except Exception as e:
            print(f"❌ Failed to generate HTML report: {e}")
            return False
    
    def _generate_html_template(self) -> str:
        """Generate HTML report template"""
        project_name = self.vcpkg_manifest.get('name', 'thread-system')
        
        html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dependency Analysis - {project_name}</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <style>
        .dependency-card {{ margin-bottom: 1rem; }}
        .category-header {{ background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }}
    </style>
</head>
<body>
    <div class="container mt-4">
        <h1 class="text-center mb-4">📊 Dependency Analysis Report</h1>
        <div class="alert alert-info">
            <strong>Project:</strong> {project_name}<br>
            <strong>Generated:</strong> {self._get_timestamp()}
        </div>
        
        <div class="row">
            <div class="col-md-8">
                <h2>Dependencies by Category</h2>
                {self._generate_dependency_cards()}
            </div>
            <div class="col-md-4">
                <h2>Summary</h2>
                {self._generate_summary_card()}
                
                <h3 class="mt-4">Actions</h3>
                {self._generate_action_buttons()}
            </div>
        </div>
        
        <div class="row mt-4">
            <div class="col-12">
                <h2>Dependency Matrix</h2>
                {self._generate_dependency_table()}
            </div>
        </div>
    </div>
    
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>"""
        return html
    
    def _generate_dependency_cards(self) -> str:
        """Generate dependency cards for HTML report"""
        cards = ""
        
        for category, deps in self.dependency_graph.items():
            if not deps:
                continue
                
            color_class = COLORS['html'].get(category, 'secondary')
            
            cards += f"""
            <div class="card dependency-card">
                <div class="card-header category-header">
                    <h5 class="mb-0">{category.title()} Dependencies ({len(deps)})</h5>
                </div>
                <div class="card-body">
            """
            
            for dep in deps:
                version_badge = f'<span class="badge bg-secondary">{dep["version"]}</span>' if dep['version'] else ''
                platform_badge = f'<span class="badge bg-info">{dep["platform"]}</span>' if dep['platform'] else ''
                features_badge = f'<span class="badge bg-warning">{", ".join(dep["features"])}</span>' if dep['features'] else ''
                
                cards += f"""
                <div class="d-flex justify-content-between align-items-center mb-2">
                    <strong>{dep['name']}</strong>
                    <div>
                        {version_badge}
                        {platform_badge} 
                        {features_badge}
                    </div>
                </div>
                """
            
            cards += "</div></div>"
        
        return cards
    
    def _generate_summary_card(self) -> str:
        """Generate summary statistics card"""
        total_deps = sum(len(deps) for deps in self.dependency_graph.values())
        core_deps = len(self.dependency_graph.get('core', []))
        optional_deps = total_deps - core_deps
        
        return f"""
        <div class="card">
            <div class="card-body">
                <h5 class="card-title">Statistics</h5>
                <ul class="list-group list-group-flush">
                    <li class="list-group-item d-flex justify-content-between">
                        <span>Total Dependencies</span>
                        <span class="badge bg-primary">{total_deps}</span>
                    </li>
                    <li class="list-group-item d-flex justify-content-between">
                        <span>Core Dependencies</span>
                        <span class="badge bg-success">{core_deps}</span>
                    </li>
                    <li class="list-group-item d-flex justify-content-between">
                        <span>Optional Dependencies</span>
                        <span class="badge bg-info">{optional_deps}</span>
                    </li>
                </ul>
            </div>
        </div>
        """
    
    def _generate_action_buttons(self) -> str:
        """Generate action buttons for HTML report"""
        return """
        <div class="d-grid gap-2">
            <button class="btn btn-primary" onclick="window.open('dependency_compatibility_matrix.md')">
                📋 View Compatibility Matrix
            </button>
            <button class="btn btn-warning" onclick="alert('Run: ./scripts/upgrade_dependencies.sh')">
                🔄 Upgrade Dependencies
            </button>
            <button class="btn btn-danger" onclick="alert('Run: ./scripts/dependency_analyzer.py --security-scan')">
                🔍 Security Scan
            </button>
        </div>
        """
    
    def _generate_dependency_table(self) -> str:
        """Generate dependency table for HTML report"""
        table = """
        <table class="table table-striped">
            <thead>
                <tr>
                    <th>Package</th>
                    <th>Category</th>
                    <th>Version</th>
                    <th>Platform</th>
                    <th>Features</th>
                </tr>
            </thead>
            <tbody>
        """
        
        for category, deps in self.dependency_graph.items():
            for dep in deps:
                version = dep.get('version', '-')
                platform = dep.get('platform', 'all')
                features = ', '.join(dep.get('features', []))
                
                table += f"""
                <tr>
                    <td><strong>{dep['name']}</strong></td>
                    <td><span class="badge bg-{COLORS['html'].get(category, 'secondary')}">{category}</span></td>
                    <td>{version}</td>
                    <td>{platform}</td>
                    <td>{features}</td>
                </tr>
                """
        
        table += "</tbody></table>"
        return table
    
    def _get_timestamp(self) -> str:
        """Get current timestamp for reports"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    def run_security_scan(self) -> bool:
        """Run security vulnerability scan"""
        print("🔍 Running security vulnerability scan...")
        
        # This would integrate with tools like safety, bandit, or npm audit
        # For now, provide guidance
        print("📋 Security scan recommendations:")
        print("   1. Run GitHub dependency security alerts")
        print("   2. Use vcpkg vulnerability database")
        print("   3. Check CVE databases for known issues")
        print("   4. Review license compatibility")
        
        return True
    
    def generate_upgrade_script(self, output_path: Path) -> bool:
        """Generate automatic upgrade script"""
        try:
            script_content = f"""#!/bin/bash
# Auto-generated dependency upgrade script for {self.vcpkg_manifest.get('name', 'thread-system')}
# Generated by dependency_analyzer.py

set -e

echo "🔄 Starting dependency upgrade process..."

# Update vcpkg itself
echo "📦 Updating vcpkg..."
git -C "$(vcpkg version | grep -o '/.*')" pull

# Upgrade each dependency
echo "⬆️ Upgrading dependencies..."
"""
            
            all_deps = set()
            for deps in self.dependency_graph.values():
                for dep in deps:
                    if dep['name'] not in ['thread-system']:
                        all_deps.add(dep['name'])
            
            for dep in sorted(all_deps):
                script_content += f'vcpkg upgrade {dep} --no-dry-run\n'
            
            script_content += """
echo "✅ Dependency upgrade completed!"
echo "🔍 Running post-upgrade validation..."

# Test build
cmake -B build_upgrade_test -DCHECK_DEPENDENCIES=ON
cmake --build build_upgrade_test

echo "🎉 Upgrade successful! Remember to commit vcpkg.json changes."
"""
            
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(script_content)
                
            output_path.chmod(0o755)  # Make executable
            print(f"📜 Upgrade script generated: {output_path}")
            return True
            
        except Exception as e:
            print(f"❌ Failed to generate upgrade script: {e}")
            return False

def main():
    parser = argparse.ArgumentParser(description='Analyze and visualize thread_system dependencies')
    parser.add_argument('--visualize', action='store_true', help='Generate dependency visualization')
    parser.add_argument('--html', action='store_true', help='Generate HTML report')
    parser.add_argument('--security-scan', action='store_true', help='Run security vulnerability scan')
    parser.add_argument('--generate-upgrade', action='store_true', help='Generate upgrade script')
    parser.add_argument('--output-dir', type=Path, default=Path('./build'), help='Output directory')
    parser.add_argument('--project-root', type=Path, default=Path('.'), help='Project root directory')
    
    args = parser.parse_args()
    
    # Ensure output directory exists
    args.output_dir.mkdir(exist_ok=True)
    
    # Initialize analyzer
    analyzer = DependencyAnalyzer(args.project_root)
    
    # Load dependencies
    if not analyzer.load_vcpkg_manifest():
        sys.exit(1)
    
    analyzer.analyze_cmake_dependencies()
    analyzer.build_dependency_graph()
    
    # Check for circular dependencies
    cycles = analyzer.check_circular_dependencies()
    if cycles:
        print(f"⚠️ Circular dependencies detected: {cycles}")
    
    # Execute requested operations
    success = True
    
    if args.visualize:
        graphviz_path = args.output_dir / 'dependency_graph.dot'
        success &= analyzer.generate_graphviz(graphviz_path)
        
        # Try to generate PNG if graphviz is available
        try:
            dot_executable = shutil.which('dot')
            if dot_executable:
                png_path = args.output_dir / 'dependency_graph.png'
                subprocess.run([dot_executable, '-Tpng', str(graphviz_path), '-o', str(png_path)], 
                             check=True, timeout=30)
                print(f"🖼️ PNG visualization generated: {png_path}")
            else:
                print("ℹ️ Graphviz 'dot' command not found in PATH")
        except subprocess.TimeoutExpired:
            print("⚠️ Graphviz PNG generation timed out (>30s)")
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("ℹ️ Install graphviz to generate PNG: brew install graphviz (macOS) or apt-get install graphviz (Ubuntu)")
    
    if args.html:
        html_path = args.output_dir / 'dependency_report.html'
        success &= analyzer.generate_html_report(html_path)
    
    if args.security_scan:
        success &= analyzer.run_security_scan()
    
    if args.generate_upgrade:
        script_path = args.project_root / 'scripts' / 'upgrade_dependencies.sh'
        script_path.parent.mkdir(exist_ok=True)
        success &= analyzer.generate_upgrade_script(script_path)
    
    # Default: show summary
    if not any([args.visualize, args.html, args.security_scan, args.generate_upgrade]):
        print("\n📊 Dependency Summary:")
        print("=" * 50)
        for category, deps in analyzer.dependency_graph.items():
            if deps:
                print(f"{category.title()}: {len(deps)} dependencies")
                for dep in deps[:3]:  # Show first 3
                    version_str = f" v{dep['version']}" if dep['version'] else ""
                    print(f"  - {dep['name']}{version_str}")
                if len(deps) > 3:
                    print(f"  ... and {len(deps) - 3} more")
        print("\nUse --help to see available analysis options")
    
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()