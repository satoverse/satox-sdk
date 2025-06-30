#!/usr/bin/env python3
"""
Benchmark Framework for Satox SDK

This module provides a framework for benchmarking various SDK operations.
It includes utilities for measuring performance, collecting metrics, and
generating reports.
"""

import time
import statistics
import json
from typing import Dict, List, Any, Callable
from dataclasses import dataclass
from datetime import datetime
import logging
from pathlib import Path

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

@dataclass
class BenchmarkResult:
    """Class for storing benchmark results"""
    operation: str
    iterations: int
    total_time: float
    avg_time: float
    min_time: float
    max_time: float
    median_time: float
    std_dev: float
    timestamp: str
    metadata: Dict[str, Any]

class BenchmarkFramework:
    def __init__(self, output_dir: str = "benchmark_results"):
        """Initialize the benchmark framework"""
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        self.results: List[BenchmarkResult] = []

    def measure(self, operation: str, func: Callable, 
                iterations: int = 100, metadata: Dict[str, Any] = None) -> BenchmarkResult:
        """Measure the performance of a function"""
        logger.info(f"Benchmarking {operation} with {iterations} iterations")
        
        times: List[float] = []
        
        # Warm-up run
        try:
            func()
        except Exception as e:
            logger.error(f"Warm-up failed: {e}")
            raise

        # Actual measurements
        for i in range(iterations):
            start_time = time.perf_counter()
            try:
                func()
            except Exception as e:
                logger.error(f"Iteration {i} failed: {e}")
                raise
            end_time = time.perf_counter()
            times.append(end_time - start_time)

        # Calculate statistics
        total_time = sum(times)
        avg_time = total_time / iterations
        min_time = min(times)
        max_time = max(times)
        median_time = statistics.median(times)
        std_dev = statistics.stdev(times) if len(times) > 1 else 0

        # Create result
        result = BenchmarkResult(
            operation=operation,
            iterations=iterations,
            total_time=total_time,
            avg_time=avg_time,
            min_time=min_time,
            max_time=max_time,
            median_time=median_time,
            std_dev=std_dev,
            timestamp=datetime.now().isoformat(),
            metadata=metadata or {}
        )

        self.results.append(result)
        return result

    def save_results(self, filename: str = None) -> str:
        """Save benchmark results to a JSON file"""
        if not filename:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"benchmark_results_{timestamp}.json"

        filepath = self.output_dir / filename
        
        # Convert results to dictionary
        results_dict = {
            "timestamp": datetime.now().isoformat(),
            "results": [
                {
                    "operation": r.operation,
                    "iterations": r.iterations,
                    "total_time": r.total_time,
                    "avg_time": r.avg_time,
                    "min_time": r.min_time,
                    "max_time": r.max_time,
                    "median_time": r.median_time,
                    "std_dev": r.std_dev,
                    "timestamp": r.timestamp,
                    "metadata": r.metadata
                }
                for r in self.results
            ]
        }

        # Save to file
        with open(filepath, 'w') as f:
            json.dump(results_dict, f, indent=2)

        logger.info(f"Results saved to {filepath}")
        return str(filepath)

    def generate_report(self, output_format: str = "markdown") -> str:
        """Generate a human-readable report of benchmark results"""
        if not self.results:
            return "No benchmark results available"

        if output_format == "markdown":
            return self._generate_markdown_report()
        else:
            raise ValueError(f"Unsupported output format: {output_format}")

    def _generate_markdown_report(self) -> str:
        """Generate a markdown report"""
        report = ["# Benchmark Results\n"]
        
        # Summary
        report.append("## Summary\n")
        report.append("| Operation | Iterations | Avg Time (s) | Min Time (s) | Max Time (s) | Std Dev (s) |")
        report.append("|-----------|------------|--------------|--------------|--------------|-------------|")
        
        for result in self.results:
            report.append(
                f"| {result.operation} | {result.iterations} | "
                f"{result.avg_time:.6f} | {result.min_time:.6f} | "
                f"{result.max_time:.6f} | {result.std_dev:.6f} |"
            )

        # Detailed Results
        report.append("\n## Detailed Results\n")
        for result in self.results:
            report.append(f"### {result.operation}\n")
            report.append(f"- Total Time: {result.total_time:.6f} seconds")
            report.append(f"- Average Time: {result.avg_time:.6f} seconds")
            report.append(f"- Minimum Time: {result.min_time:.6f} seconds")
            report.append(f"- Maximum Time: {result.max_time:.6f} seconds")
            report.append(f"- Median Time: {result.median_time:.6f} seconds")
            report.append(f"- Standard Deviation: {result.std_dev:.6f} seconds")
            
            if result.metadata:
                report.append("\nMetadata:")
                for key, value in result.metadata.items():
                    report.append(f"- {key}: {value}")
            report.append("")

        return "\n".join(report)

def benchmark_decorator(iterations: int = 100):
    """Decorator for benchmarking functions"""
    def decorator(func: Callable):
        def wrapper(*args, **kwargs):
            framework = BenchmarkFramework()
            result = framework.measure(
                operation=func.__name__,
                func=lambda: func(*args, **kwargs),
                iterations=iterations
            )
            return result
        return wrapper
    return decorator 