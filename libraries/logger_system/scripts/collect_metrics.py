#!/usr/bin/env python3
"""
CI/CD Metrics Collection Script for Logger System
Collects build, test, and performance metrics from CI/CD runs
"""

import json
import os
import sys
import subprocess
import argparse
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any, Optional

class MetricsCollector:
    """Collects and processes CI/CD metrics"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.metrics_dir = project_root / "metrics"
        self.metrics_dir.mkdir(exist_ok=True)
        
    def collect_test_results(self, test_results_path: Path) -> Dict[str, Any]:
        """Parse test results from JUnit XML or JSON format"""
        results = {
            "total": 0,
            "passed": 0,
            "failed": 0,
            "skipped": 0,
            "duration": 0.0,
            "timestamp": datetime.now().isoformat()
        }
        
        if test_results_path.exists():
            # Parse test results based on format
            if test_results_path.suffix == ".xml":
                results = self._parse_junit_xml(test_results_path)
            elif test_results_path.suffix == ".json":
                with open(test_results_path, 'r') as f:
                    results = json.load(f)
                    
        return results
    
    def _parse_junit_xml(self, xml_path: Path) -> Dict[str, Any]:
        """Parse JUnit XML test results"""
        try:
            import xml.etree.ElementTree as ET
            tree = ET.parse(xml_path)
            root = tree.getroot()
            
            # Parse test suite statistics
            testsuite = root.find('.//testsuite')
            if testsuite is not None:
                return {
                    "total": int(testsuite.get('tests', 0)),
                    "passed": int(testsuite.get('tests', 0)) - 
                             int(testsuite.get('failures', 0)) - 
                             int(testsuite.get('errors', 0)) -
                             int(testsuite.get('skipped', 0)),
                    "failed": int(testsuite.get('failures', 0)) + 
                             int(testsuite.get('errors', 0)),
                    "skipped": int(testsuite.get('skipped', 0)),
                    "duration": float(testsuite.get('time', 0)),
                    "timestamp": datetime.now().isoformat()
                }
        except Exception as e:
            print(f"Error parsing JUnit XML: {e}")
            
        return {"error": "Failed to parse test results"}
    
    def collect_coverage_data(self, coverage_file: Path) -> Dict[str, Any]:
        """Collect code coverage metrics"""
        coverage = {
            "line_coverage": 0.0,
            "branch_coverage": 0.0,
            "function_coverage": 0.0,
            "timestamp": datetime.now().isoformat()
        }
        
        if coverage_file.exists():
            if coverage_file.suffix == ".json":
                with open(coverage_file, 'r') as f:
                    data = json.load(f)
                    coverage.update(data)
            elif coverage_file.suffix == ".xml":
                coverage = self._parse_coverage_xml(coverage_file)
                
        return coverage
    
    def _parse_coverage_xml(self, xml_path: Path) -> Dict[str, Any]:
        """Parse coverage XML (Cobertura format)"""
        try:
            import xml.etree.ElementTree as ET
            tree = ET.parse(xml_path)
            root = tree.getroot()
            
            # Extract coverage percentages
            return {
                "line_coverage": float(root.get('line-rate', 0)) * 100,
                "branch_coverage": float(root.get('branch-rate', 0)) * 100,
                "function_coverage": 0.0,  # Not always available in Cobertura
                "lines_covered": int(root.get('lines-covered', 0)),
                "lines_valid": int(root.get('lines-valid', 0)),
                "timestamp": datetime.now().isoformat()
            }
        except Exception as e:
            print(f"Error parsing coverage XML: {e}")
            
        return {"error": "Failed to parse coverage data"}
    
    def collect_build_metrics(self, build_log: Optional[Path] = None) -> Dict[str, Any]:
        """Extract build time and status from build logs"""
        metrics = {
            "build_time": 0.0,
            "status": "unknown",
            "warnings": 0,
            "errors": 0,
            "timestamp": datetime.now().isoformat()
        }
        
        if build_log and build_log.exists():
            with open(build_log, 'r') as f:
                content = f.read()
                
                # Extract build time (simple pattern matching)
                import re
                time_match = re.search(r'Build time: ([\d.]+)s', content)
                if time_match:
                    metrics["build_time"] = float(time_match.group(1))
                
                # Count warnings and errors
                metrics["warnings"] = len(re.findall(r'warning:', content, re.IGNORECASE))
                metrics["errors"] = len(re.findall(r'error:', content, re.IGNORECASE))
                
                # Determine status
                if "BUILD SUCCESSFUL" in content or "Build succeeded" in content:
                    metrics["status"] = "success"
                elif "BUILD FAILED" in content or "Build failed" in content:
                    metrics["status"] = "failed"
                    
        return metrics
    
    def collect_performance_metrics(self, benchmark_results: Path) -> Dict[str, Any]:
        """Parse benchmark results"""
        metrics = {
            "p99_latency_us": 0.0,
            "throughput_msg_per_sec": 0.0,
            "memory_usage_mb": 0.0,
            "cpu_usage_percent": 0.0,
            "timestamp": datetime.now().isoformat()
        }
        
        if benchmark_results.exists():
            with open(benchmark_results, 'r') as f:
                data = json.load(f) if benchmark_results.suffix == ".json" else {}
                metrics.update(data)
                
        return metrics
    
    def generate_summary(self, all_metrics: Dict[str, Any]) -> str:
        """Generate a markdown summary of metrics"""
        summary = []
        summary.append("# CI/CD Metrics Summary\n")
        summary.append(f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        
        # Test results
        if "test_results" in all_metrics:
            tests = all_metrics["test_results"]
            summary.append("\n## Test Results")
            summary.append(f"- Total: {tests.get('total', 0)}")
            summary.append(f"- Passed: {tests.get('passed', 0)}")
            summary.append(f"- Failed: {tests.get('failed', 0)}")
            summary.append(f"- Skipped: {tests.get('skipped', 0)}")
            summary.append(f"- Duration: {tests.get('duration', 0):.2f}s")
        
        # Coverage
        if "coverage" in all_metrics:
            cov = all_metrics["coverage"]
            summary.append("\n## Code Coverage")
            summary.append(f"- Line Coverage: {cov.get('line_coverage', 0):.1f}%")
            summary.append(f"- Branch Coverage: {cov.get('branch_coverage', 0):.1f}%")
            summary.append(f"- Function Coverage: {cov.get('function_coverage', 0):.1f}%")
        
        # Build metrics
        if "build" in all_metrics:
            build = all_metrics["build"]
            summary.append("\n## Build Metrics")
            summary.append(f"- Status: {build.get('status', 'unknown')}")
            summary.append(f"- Build Time: {build.get('build_time', 0):.1f}s")
            summary.append(f"- Warnings: {build.get('warnings', 0)}")
            summary.append(f"- Errors: {build.get('errors', 0)}")
        
        # Performance
        if "performance" in all_metrics:
            perf = all_metrics["performance"]
            summary.append("\n## Performance Metrics")
            summary.append(f"- P99 Latency: {perf.get('p99_latency_us', 0):.1f}μs")
            summary.append(f"- Throughput: {perf.get('throughput_msg_per_sec', 0):,.0f} msg/s")
            summary.append(f"- Memory Usage: {perf.get('memory_usage_mb', 0):.1f}MB")
            summary.append(f"- CPU Usage: {perf.get('cpu_usage_percent', 0):.1f}%")
        
        return "\n".join(summary)
    
    def save_metrics(self, metrics: Dict[str, Any], filename: str = "metrics.json"):
        """Save metrics to file"""
        output_path = self.metrics_dir / filename
        with open(output_path, 'w') as f:
            json.dump(metrics, f, indent=2)
        print(f"Metrics saved to: {output_path}")
        
        # Also save summary
        summary_path = self.metrics_dir / "summary.md"
        with open(summary_path, 'w') as f:
            f.write(self.generate_summary(metrics))
        print(f"Summary saved to: {summary_path}")
    
    def update_dashboard(self, metrics: Dict[str, Any]):
        """Update the CI/CD dashboard with latest metrics"""
        dashboard_path = self.project_root / "docs" / "CI_CD_DASHBOARD.md"
        
        if not dashboard_path.exists():
            print(f"Dashboard not found at {dashboard_path}")
            return
        
        # This would normally update specific sections of the dashboard
        # For now, we'll just append a timestamp
        print(f"Dashboard update would be applied to: {dashboard_path}")
        
def main():
    parser = argparse.ArgumentParser(description="Collect CI/CD metrics")
    parser.add_argument("--project-root", type=Path, default=Path.cwd(),
                       help="Project root directory")
    parser.add_argument("--test-results", type=Path,
                       help="Path to test results file")
    parser.add_argument("--coverage-file", type=Path,
                       help="Path to coverage file")
    parser.add_argument("--build-log", type=Path,
                       help="Path to build log")
    parser.add_argument("--benchmark-results", type=Path,
                       help="Path to benchmark results")
    parser.add_argument("--output", type=str, default="metrics.json",
                       help="Output filename")
    parser.add_argument("--update-dashboard", action="store_true",
                       help="Update the CI/CD dashboard")
    
    args = parser.parse_args()
    
    collector = MetricsCollector(args.project_root)
    
    all_metrics = {}
    
    # Collect various metrics
    if args.test_results:
        all_metrics["test_results"] = collector.collect_test_results(args.test_results)
    
    if args.coverage_file:
        all_metrics["coverage"] = collector.collect_coverage_data(args.coverage_file)
    
    if args.build_log:
        all_metrics["build"] = collector.collect_build_metrics(args.build_log)
    
    if args.benchmark_results:
        all_metrics["performance"] = collector.collect_performance_metrics(args.benchmark_results)
    
    # Save metrics
    if all_metrics:
        collector.save_metrics(all_metrics, args.output)
        
        if args.update_dashboard:
            collector.update_dashboard(all_metrics)
    else:
        print("No metrics collected. Please provide input files.")
        sys.exit(1)

if __name__ == "__main__":
    main()