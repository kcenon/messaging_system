#!/usr/bin/env python3
"""
Documentation Coverage Analysis Tool for Thread System
Analyzes source code and calculates documentation coverage metrics
"""

import os
import re
import sys
import json
from pathlib import Path
from dataclasses import dataclass
from typing import List, Dict, Tuple


@dataclass
class FileStats:
    """Statistics for a single file"""
    filename: str
    total_functions: int = 0
    documented_functions: int = 0
    total_classes: int = 0
    documented_classes: int = 0
    total_lines: int = 0
    comment_lines: int = 0
    doxygen_blocks: int = 0


class DocCoverageAnalyzer:
    """Analyzes documentation coverage for C++ projects"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.source_extensions = {'.h', '.hpp', '.cpp', '.cc', '.cxx'}
        self.stats: Dict[str, FileStats] = {}
        
    def analyze(self) -> Dict[str, any]:
        """Analyze the entire project"""
        print("Analyzing documentation coverage...")
        
        # Find all source files
        source_files = self._find_source_files()
        
        # Analyze each file
        for file_path in source_files:
            self._analyze_file(file_path)
            
        # Calculate overall statistics
        return self._calculate_summary()
        
    def _find_source_files(self) -> List[Path]:
        """Find all C++ source files in the project"""
        source_files = []
        sources_dir = self.project_root / "sources"
        
        if sources_dir.exists():
            for file_path in sources_dir.rglob("*"):
                if file_path.suffix in self.source_extensions:
                    source_files.append(file_path)
                    
        return source_files
        
    def _analyze_file(self, file_path: Path):
        """Analyze a single source file"""
        stats = FileStats(filename=str(file_path.relative_to(self.project_root)))
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                lines = content.split('\n')
                
            stats.total_lines = len(lines)
            
            # Count comment lines
            in_block_comment = False
            for line in lines:
                stripped = line.strip()
                if '/*' in stripped:
                    in_block_comment = True
                if in_block_comment or stripped.startswith('//'):
                    stats.comment_lines += 1
                if '*/' in stripped:
                    in_block_comment = False
                    
            # Count Doxygen blocks
            stats.doxygen_blocks = len(re.findall(r'/\*\*[\s\S]*?\*/', content))
            stats.doxygen_blocks += len(re.findall(r'///.*', content))
            
            # Count functions and their documentation
            # Simple pattern matching - not perfect but good enough
            function_pattern = r'^\s*(?:[\w\s\*&:<>]+)\s+(\w+)\s*\([^)]*\)\s*(?:const)?\s*(?:override)?\s*[{;]'
            functions = re.findall(function_pattern, content, re.MULTILINE)
            stats.total_functions = len(functions)
            
            # Check for documented functions (preceded by doxygen comment)
            doc_function_pattern = r'(?:/\*\*[\s\S]*?\*/|///.*)\s*\n\s*(?:[\w\s\*&:<>]+)\s+(\w+)\s*\('
            documented_functions = re.findall(doc_function_pattern, content)
            stats.documented_functions = len(documented_functions)
            
            # Count classes and their documentation
            class_pattern = r'^\s*(?:class|struct)\s+(\w+)'
            classes = re.findall(class_pattern, content, re.MULTILINE)
            stats.total_classes = len(classes)
            
            # Check for documented classes
            doc_class_pattern = r'(?:/\*\*[\s\S]*?\*/|///.*)\s*\n\s*(?:class|struct)\s+(\w+)'
            documented_classes = re.findall(doc_class_pattern, content)
            stats.documented_classes = len(documented_classes)
            
        except Exception as e:
            print(f"Error analyzing {file_path}: {e}")
            
        self.stats[stats.filename] = stats
        
    def _calculate_summary(self) -> Dict[str, any]:
        """Calculate summary statistics"""
        total_files = len(self.stats)
        total_functions = sum(s.total_functions for s in self.stats.values())
        documented_functions = sum(s.documented_functions for s in self.stats.values())
        total_classes = sum(s.total_classes for s in self.stats.values())
        documented_classes = sum(s.documented_classes for s in self.stats.values())
        total_lines = sum(s.total_lines for s in self.stats.values())
        comment_lines = sum(s.comment_lines for s in self.stats.values())
        doxygen_blocks = sum(s.doxygen_blocks for s in self.stats.values())
        
        # Files with any documentation
        documented_files = sum(1 for s in self.stats.values() if s.doxygen_blocks > 0)
        
        summary = {
            'total_files': total_files,
            'documented_files': documented_files,
            'file_coverage': (documented_files / total_files * 100) if total_files > 0 else 0,
            'total_functions': total_functions,
            'documented_functions': documented_functions,
            'function_coverage': (documented_functions / total_functions * 100) if total_functions > 0 else 0,
            'total_classes': total_classes,
            'documented_classes': documented_classes,
            'class_coverage': (documented_classes / total_classes * 100) if total_classes > 0 else 0,
            'total_lines': total_lines,
            'comment_lines': comment_lines,
            'comment_ratio': (comment_lines / total_lines * 100) if total_lines > 0 else 0,
            'doxygen_blocks': doxygen_blocks,
            'files': self.stats
        }
        
        return summary
        
    def generate_report(self, output_format: str = 'text'):
        """Generate a coverage report"""
        summary = self.analyze()
        
        if output_format == 'json':
            print(json.dumps(summary, indent=2))
        elif output_format == 'markdown':
            self._generate_markdown_report(summary)
        else:
            self._generate_text_report(summary)
            
    def _generate_text_report(self, summary: Dict):
        """Generate a text format report"""
        print("\n" + "="*60)
        print("Thread System Documentation Coverage Report")
        print("="*60)
        
        print(f"\n📊 Overall Statistics:")
        print(f"  Total Files: {summary['total_files']}")
        print(f"  Documented Files: {summary['documented_files']} ({summary['file_coverage']:.1f}%)")
        print(f"  Total Functions: {summary['total_functions']}")
        print(f"  Documented Functions: {summary['documented_functions']} ({summary['function_coverage']:.1f}%)")
        print(f"  Total Classes/Structs: {summary['total_classes']}")
        print(f"  Documented Classes: {summary['documented_classes']} ({summary['class_coverage']:.1f}%)")
        print(f"  Comment Ratio: {summary['comment_ratio']:.1f}%")
        print(f"  Doxygen Blocks: {summary['doxygen_blocks']}")
        
        # Find files needing documentation
        undocumented = [(f, s) for f, s in summary['files'].items() if s.doxygen_blocks == 0]
        if undocumented:
            print(f"\n⚠️  Files needing documentation ({len(undocumented)}):")
            for filename, _ in undocumented[:10]:  # Show first 10
                print(f"    - {filename}")
            if len(undocumented) > 10:
                print(f"    ... and {len(undocumented) - 10} more")
                
        # Coverage grade
        overall_coverage = (summary['file_coverage'] + summary['function_coverage'] + 
                          summary['class_coverage']) / 3
        
        print(f"\n📈 Overall Documentation Score: ", end="")
        if overall_coverage >= 90:
            print(f"A ({overall_coverage:.1f}%) ✅")
        elif overall_coverage >= 80:
            print(f"B ({overall_coverage:.1f}%) ✅")
        elif overall_coverage >= 70:
            print(f"C ({overall_coverage:.1f}%) ⚠️")
        elif overall_coverage >= 60:
            print(f"D ({overall_coverage:.1f}%) ⚠️")
        else:
            print(f"F ({overall_coverage:.1f}%) ❌")
            
    def _generate_markdown_report(self, summary: Dict):
        """Generate a markdown format report"""
        print("# Thread System Documentation Coverage Report\n")
        
        print("## Summary\n")
        print(f"| Metric | Count | Coverage |")
        print(f"|--------|-------|----------|")
        print(f"| Files | {summary['total_files']} | {summary['file_coverage']:.1f}% |")
        print(f"| Functions | {summary['total_functions']} | {summary['function_coverage']:.1f}% |")
        print(f"| Classes | {summary['total_classes']} | {summary['class_coverage']:.1f}% |")
        print(f"| Comment Ratio | {summary['total_lines']} lines | {summary['comment_ratio']:.1f}% |")
        
        print("\n## Coverage by Module\n")
        
        # Group by directory
        modules = {}
        for filename, stats in summary['files'].items():
            module = filename.split('/')[1] if '/' in filename else 'root'
            if module not in modules:
                modules[module] = {'files': 0, 'documented': 0}
            modules[module]['files'] += 1
            if stats.doxygen_blocks > 0:
                modules[module]['documented'] += 1
                
        print("| Module | Files | Documented | Coverage |")
        print("|--------|-------|------------|----------|")
        for module, data in sorted(modules.items()):
            coverage = (data['documented'] / data['files'] * 100) if data['files'] > 0 else 0
            print(f"| {module} | {data['files']} | {data['documented']} | {coverage:.1f}% |")


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        project_root = Path(__file__).parent.parent
    else:
        project_root = Path(sys.argv[1])
        
    if not project_root.exists():
        print(f"Error: Project root {project_root} does not exist")
        sys.exit(1)
        
    format_type = 'text'
    if len(sys.argv) > 2:
        format_type = sys.argv[2]
        
    analyzer = DocCoverageAnalyzer(project_root)
    analyzer.generate_report(format_type)


if __name__ == "__main__":
    main()