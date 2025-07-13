#!/usr/bin/env python3
"""
Backward compatibility wrapper for the messaging system Python module.

This file provides compatibility with the old structure.
Please use the new module structure:
    from python.messaging_system import Container, Value, MessagingClient
"""

import sys
import os

# Add python directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python'))

# Import from new structure
from messaging_system.container import Container as container, Value as value
from messaging_system.network import MessagingClient as messaging_client

# Export for backward compatibility
__all__ = ['container', 'value', 'messaging_client']

if __name__ == "__main__":
    print("Messaging System Python Module")
    print("==============================")
    print()
    print("This is a compatibility wrapper. Please use the new module structure:")
    print()
    print("  cd python")
    print("  pip install -e .")
    print()
    print("Then import as:")
    print("  from messaging_system import Container, Value, MessagingClient")
    print()
    print("Or run samples:")
    print("  python -m messaging_system.samples.container_demo")
    print("  python -m messaging_system.samples.simple_client <server_ip> <port>")
    print()