#!/usr/bin/env python3
import json
import sys

def load(path):
    with open(path, 'r') as f:
        return json.load(f)

def index_by_name(results):
    out = {}
    for r in results.get('benchmarks', []):
        out[r.get('name', '')] = r
    return out

def main():
    if len(sys.argv) < 3:
        print("Usage: benchmark_compare.py <baseline.json> <current.json>")
        sys.exit(2)
    base = index_by_name(load(sys.argv[1]))
    curr = index_by_name(load(sys.argv[2]))
    print("Benchmark comparison (time lower is better):")
    for name, b in base.items():
        c = curr.get(name)
        if not c:
            continue
        bt = b.get('cpu_time', b.get('real_time'))
        ct = c.get('cpu_time', c.get('real_time'))
        if bt and ct:
            delta = (ct - bt) / bt * 100.0
            print(f"- {name}: baseline={bt:.2f}, current={ct:.2f}, change={delta:+.2f}%")

if __name__ == '__main__':
    main()

