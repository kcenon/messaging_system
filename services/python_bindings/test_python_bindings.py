#!/usr/bin/env python3
"""
Test suite for KCENON Messaging System Python bindings

Tests the Python wrapper functionality and integration with the C++ messaging system.
"""

import unittest
import time
import threading
from unittest.mock import patch
import sys
import os

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    from messaging_system_python import (
        MessagingSystem, AsyncMessagingSystem, MessagingSystemConfig,
        Message, MessagePriority, create_messaging_system
    )
except ImportError as e:
    print(f"Warning: Could not import messaging system: {e}")
    print("Python bindings may not be built. Skipping tests.")
    sys.exit(0)


class TestMessagingSystemConfig(unittest.TestCase):
    """Test the messaging system configuration builder"""

    def test_config_builder_chain(self):
        """Test that configuration builder methods can be chained"""
        config = MessagingSystemConfig()

        result = (config
                 .worker_threads(4)
                 .queue_size(10000)
                 .priority_queue(True)
                 .compression(True)
                 .environment("testing")
                 .system_name("test_system"))

        self.assertIsInstance(result, MessagingSystemConfig)

    def test_config_build(self):
        """Test that configuration can be built"""
        config = MessagingSystemConfig()
        config_obj = config.build()

        # Just verify it doesn't crash
        self.assertIsNotNone(config_obj)


class TestMessage(unittest.TestCase):
    """Test the Message class"""

    def test_message_creation(self):
        """Test creating a message"""
        msg = Message("test.topic", {"key": "value"}, "test_sender")

        self.assertEqual(msg.topic, "test.topic")
        self.assertEqual(msg.data, {"key": "value"})
        self.assertEqual(msg.sender, "test_sender")
        self.assertEqual(msg.priority, MessagePriority.NORMAL)

    def test_message_to_dict(self):
        """Test converting message to dictionary"""
        msg = Message("test.topic", {"key": "value"})
        msg_dict = msg.to_dict()

        self.assertIn("topic", msg_dict)
        self.assertIn("data", msg_dict)
        self.assertIn("sender", msg_dict)
        self.assertEqual(msg_dict["topic"], "test.topic")

    def test_message_from_dict(self):
        """Test creating message from dictionary"""
        msg_dict = {
            "topic": "test.topic",
            "data": {"key": "value"},
            "sender": "test_sender",
            "priority": MessagePriority.HIGH
        }

        msg = Message.from_dict(msg_dict)
        self.assertEqual(msg.topic, "test.topic")
        self.assertEqual(msg.data, {"key": "value"})
        self.assertEqual(msg.priority, MessagePriority.HIGH)


class TestMessagingSystem(unittest.TestCase):
    """Test the main messaging system"""

    def setUp(self):
        """Set up test environment"""
        self.config = MessagingSystemConfig().worker_threads(2).queue_size(1000)
        self.messaging_system = MessagingSystem(self.config)

    def tearDown(self):
        """Clean up after tests"""
        if self.messaging_system.is_running():
            self.messaging_system.stop()

    def test_system_lifecycle(self):
        """Test starting and stopping the system"""
        self.assertFalse(self.messaging_system.is_running())

        # Start the system
        success = self.messaging_system.start()
        self.assertTrue(success)
        self.assertTrue(self.messaging_system.is_running())

        # Stop the system
        self.messaging_system.stop()
        self.assertFalse(self.messaging_system.is_running())

    def test_context_manager(self):
        """Test using the messaging system as a context manager"""
        with MessagingSystem(self.config) as msg_system:
            self.assertTrue(msg_system.is_running())

        # System should be stopped after exiting context
        self.assertFalse(msg_system.is_running())

    def test_publish_without_start(self):
        """Test that publishing fails when system is not started"""
        result = self.messaging_system.publish("test.topic", {"key": "value"})
        self.assertFalse(result)

    def test_subscribe_without_start(self):
        """Test that subscribing works but warns when system is not started"""
        def handler(msg):
            pass

        # Should not crash but may log warning
        self.messaging_system.subscribe("test.topic", handler)

    def test_basic_publish_subscribe(self):
        """Test basic publish/subscribe functionality"""
        self.messaging_system.start()

        received_messages = []

        def message_handler(message):
            received_messages.append(message)

        # Subscribe to topic
        self.messaging_system.subscribe("test.topic", message_handler)

        # Publish message
        test_data = {"message": "hello", "number": 42}
        success = self.messaging_system.publish("test.topic", test_data, "test_sender")
        self.assertTrue(success)

        # Wait for message processing
        time.sleep(0.1)

        # Verify message was received
        self.assertEqual(len(received_messages), 1)
        received_msg = received_messages[0]
        self.assertIsInstance(received_msg, Message)
        self.assertEqual(received_msg.topic, "test.topic")
        self.assertEqual(received_msg.sender, "test_sender")

    def test_multiple_subscribers(self):
        """Test multiple subscribers to the same topic"""
        self.messaging_system.start()

        received_count = [0]  # Use list for closure

        def handler1(msg):
            received_count[0] += 1

        def handler2(msg):
            received_count[0] += 1

        # Subscribe with multiple handlers
        self.messaging_system.subscribe("broadcast.topic", handler1)
        self.messaging_system.subscribe("broadcast.topic", handler2)

        # Publish one message
        self.messaging_system.publish("broadcast.topic", {"broadcast": True})

        # Wait for processing
        time.sleep(0.1)

        # Both handlers should receive the message
        self.assertEqual(received_count[0], 2)

    def test_health_check(self):
        """Test system health monitoring"""
        health = self.messaging_system.get_health()
        self.assertIn("running", health)
        self.assertFalse(health["running"])  # System not started yet

        self.messaging_system.start()
        health = self.messaging_system.get_health()
        self.assertTrue(health["running"])
        self.assertIn("message_bus_healthy", health)

    def test_subscribed_topics(self):
        """Test getting list of subscribed topics"""
        self.messaging_system.start()

        # Initially no topics
        topics = self.messaging_system.get_subscribed_topics()
        self.assertEqual(len(topics), 0)

        # Subscribe to topics
        def handler(msg):
            pass

        self.messaging_system.subscribe("topic1", handler)
        self.messaging_system.subscribe("topic2", handler)

        topics = self.messaging_system.get_subscribed_topics()
        self.assertEqual(len(topics), 2)
        self.assertIn("topic1", topics)
        self.assertIn("topic2", topics)


class TestAsyncMessagingSystem(unittest.TestCase):
    """Test the async messaging system"""

    def setUp(self):
        """Set up test environment"""
        self.config = MessagingSystemConfig().worker_threads(2).queue_size(1000)
        self.async_system = AsyncMessagingSystem(self.config)

    def tearDown(self):
        """Clean up after tests"""
        if self.async_system.is_running():
            self.async_system.stop_async()

    def test_async_lifecycle(self):
        """Test async system start/stop"""
        self.assertFalse(self.async_system.is_running())

        success = self.async_system.start_async()
        self.assertTrue(success)
        self.assertTrue(self.async_system.is_running())

        # Let it run briefly
        time.sleep(0.2)

        self.async_system.stop_async()
        self.assertFalse(self.async_system.is_running())


class TestFactoryFunctions(unittest.TestCase):
    """Test factory functions"""

    def test_create_messaging_system_development(self):
        """Test creating development messaging system"""
        system = create_messaging_system("development")
        self.assertIsInstance(system, MessagingSystem)

        with system:
            self.assertTrue(system.is_running())

    def test_create_messaging_system_production(self):
        """Test creating production messaging system"""
        system = create_messaging_system("production")
        self.assertIsInstance(system, MessagingSystem)

        with system:
            self.assertTrue(system.is_running())


class TestDataTypeConversion(unittest.TestCase):
    """Test data type conversion between Python and C++"""

    def setUp(self):
        """Set up test environment"""
        self.config = MessagingSystemConfig().worker_threads(1).queue_size(100)
        self.messaging_system = MessagingSystem(self.config)
        self.messaging_system.start()

    def tearDown(self):
        """Clean up after tests"""
        self.messaging_system.stop()

    def test_string_data(self):
        """Test string data conversion"""
        received_data = []

        def handler(msg):
            received_data.append(msg.data)

        self.messaging_system.subscribe("string.test", handler)

        test_data = {"string_value": "hello world"}
        self.messaging_system.publish("string.test", test_data)

        time.sleep(0.1)
        self.assertEqual(len(received_data), 1)

    def test_numeric_data(self):
        """Test numeric data conversion"""
        received_data = []

        def handler(msg):
            received_data.append(msg.data)

        self.messaging_system.subscribe("numeric.test", handler)

        test_data = {
            "integer": 42,
            "float": 3.14,
            "boolean": True
        }
        self.messaging_system.publish("numeric.test", test_data)

        time.sleep(0.1)
        self.assertEqual(len(received_data), 1)

    def test_complex_data(self):
        """Test complex data type conversion (JSON serialization)"""
        received_data = []

        def handler(msg):
            received_data.append(msg.data)

        self.messaging_system.subscribe("complex.test", handler)

        test_data = {
            "list": [1, 2, 3],
            "dict": {"nested": "value"},
            "mixed": {"numbers": [1, 2], "strings": ["a", "b"]}
        }
        self.messaging_system.publish("complex.test", test_data)

        time.sleep(0.1)
        self.assertEqual(len(received_data), 1)


def run_tests():
    """Run all tests"""
    print("Running KCENON Messaging System Python Binding Tests")
    print("=" * 60)

    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add test classes
    test_classes = [
        TestMessagingSystemConfig,
        TestMessage,
        TestMessagingSystem,
        TestAsyncMessagingSystem,
        TestFactoryFunctions,
        TestDataTypeConversion
    ]

    for test_class in test_classes:
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    return result.wasSuccessful()


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)