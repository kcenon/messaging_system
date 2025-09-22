/**
 * BSD 3-Clause License
 * Copyright (c) 2025, Database System Project
 *
 * ORM Framework Demonstration
 * Shows C++20 concepts-based entity definitions and type-safe operations
 */

#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include "database/database_manager.h"
#include "database/orm/entity.h"

using namespace database;
using namespace database::orm;

// Example Entity: User
class User : public entity_base
{
    ENTITY_TABLE("users")

    // Define fields with constraints
    ENTITY_FIELD(int64_t, id, primary_key() | auto_increment())
    ENTITY_FIELD(std::string, username, not_null() | unique() | index("idx_username"))
    ENTITY_FIELD(std::string, email, not_null() | unique())
    ENTITY_FIELD(std::string, full_name, not_null())
    ENTITY_FIELD(std::chrono::system_clock::time_point, created_at, default_now())
    ENTITY_FIELD(bool, is_active, not_null())

    ENTITY_METADATA()

public:
    User() {
        // Set default values
        is_active = true;
        created_at = std::chrono::system_clock::now();
    }

    // Custom validation method
    bool is_valid() const {
        return !username.get().empty() &&
               !email.get().empty() &&
               email.get().find('@') != std::string::npos;
    }

    // Display method
    void print_info() const {
        std::cout << "User ID: " << id.get()
                  << ", Username: " << username.get()
                  << ", Email: " << email.get()
                  << ", Active: " << (is_active.get() ? "Yes" : "No") << std::endl;
    }
};

// Implement metadata initialization (required by ENTITY_METADATA macro)
void User::initialize_metadata() {
    metadata_.add_field(id_field());
    metadata_.add_field(username_field());
    metadata_.add_field(email_field());
    metadata_.add_field(full_name_field());
    metadata_.add_field(created_at_field());
    metadata_.add_field(is_active_field());
}

// Example Entity: Product
class Product : public entity_base
{
    ENTITY_TABLE("products")

    ENTITY_FIELD(int64_t, id, primary_key() | auto_increment())
    ENTITY_FIELD(std::string, name, not_null() | index("idx_product_name"))
    ENTITY_FIELD(std::string, description)
    ENTITY_FIELD(double, price, not_null())
    ENTITY_FIELD(int32_t, stock_quantity, not_null())
    ENTITY_FIELD(bool, is_available, not_null())

    ENTITY_METADATA()

public:
    Product() {
        is_available = true;
        stock_quantity = 0;
        price = 0.0;
    }

    bool is_in_stock() const {
        return stock_quantity.get() > 0 && is_available.get();
    }

    void print_info() const {
        std::cout << "Product: " << name.get()
                  << ", Price: $" << price.get()
                  << ", Stock: " << stock_quantity.get()
                  << ", Available: " << (is_available.get() ? "Yes" : "No") << std::endl;
    }
};

void Product::initialize_metadata() {
    metadata_.add_field(id_field());
    metadata_.add_field(name_field());
    metadata_.add_field(description_field());
    metadata_.add_field(price_field());
    metadata_.add_field(stock_quantity_field());
    metadata_.add_field(is_available_field());
}

void demonstrate_entity_definition() {
    std::cout << "=== Entity Definition Demonstration ===" << std::endl;

    // Create User entity
    User user;
    user.username = "john_doe";
    user.email = "john@example.com";
    user.full_name = "John Doe";

    std::cout << "Created user entity:" << std::endl;
    user.print_info();
    std::cout << "Is valid: " << (user.is_valid() ? "Yes" : "No") << std::endl;

    // Access field metadata
    const auto& user_meta = user.get_metadata();
    std::cout << "\nUser table metadata:" << std::endl;
    std::cout << "Table name: " << user_meta.table_name() << std::endl;
    std::cout << "Field count: " << user_meta.fields().size() << std::endl;

    for (const auto& field : user_meta.fields()) {
        std::cout << "  - " << field.name() << " (" << field.type_name() << ")";
        if (field.is_primary_key()) std::cout << " [PRIMARY KEY]";
        if (field.is_unique()) std::cout << " [UNIQUE]";
        if (field.is_not_null()) std::cout << " [NOT NULL]";
        if (field.has_index()) std::cout << " [INDEXED]";
        std::cout << std::endl;
    }
}

void demonstrate_schema_management() {
    std::cout << "\n=== Schema Management Demonstration ===" << std::endl;

    try {
        // Register entities with the manager
        std::cout << "Registering entities..." << std::endl;
        entity_manager::instance().register_entity<User>();
        entity_manager::instance().register_entity<Product>();

        // Get metadata for registered entities
        const auto& user_metadata = entity_manager::instance().get_metadata<User>();
        const auto& product_metadata = entity_manager::instance().get_metadata<Product>();

        std::cout << "Registered entities:" << std::endl;
        std::cout << "  - " << user_metadata.table_name() << std::endl;
        std::cout << "  - " << product_metadata.table_name() << std::endl;

        // Generate CREATE TABLE SQL (would be used with actual database)
        std::cout << "\nGenerated SQL for User table:" << std::endl;
        std::cout << user_metadata.create_table_sql() << std::endl;

        std::cout << "\nGenerated SQL for Product table:" << std::endl;
        std::cout << product_metadata.create_table_sql() << std::endl;

        // In a real application, you would:
        // auto db = get_database_connection();
        // entity_manager::instance().create_tables(db);

        std::cout << "\nNote: In production, call entity_manager::instance().create_tables(db) to create actual tables." << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error in schema management: " << e.what() << std::endl;
    }
}

void demonstrate_type_safety() {
    std::cout << "\n=== Type Safety Demonstration ===" << std::endl;

    // Create entities with type-safe field access
    User user;
    Product product;

    // Type-safe assignments
    user.id = 1;  // int64_t
    user.username = "alice";  // std::string
    user.is_active = true;  // bool

    product.id = 100;
    product.price = 29.99;  // double
    product.stock_quantity = 50;  // int32_t

    std::cout << "Type-safe field access:" << std::endl;
    std::cout << "User ID (int64_t): " << user.id.get() << std::endl;
    std::cout << "Product price (double): " << product.price.get() << std::endl;

    // Demonstrate field metadata access
    std::cout << "\nField metadata access:" << std::endl;
    std::cout << "Username field name: " << user.username.metadata().name() << std::endl;
    std::cout << "Username constraints: ";
    if (user.username.metadata().is_unique()) std::cout << "UNIQUE ";
    if (user.username.metadata().is_not_null()) std::cout << "NOT NULL ";
    if (user.username.metadata().has_index()) std::cout << "INDEXED ";
    std::cout << std::endl;
}

void demonstrate_entity_queries() {
    std::cout << "\n=== Entity Query Demonstration ===" << std::endl;

    try {
        // Note: This demonstrates the API structure
        // In production, you would have an actual database connection
        std::cout << "Query API demonstration (requires database connection):" << std::endl;

        // Example query building (conceptual)
        std::cout << "\nExample query operations:" << std::endl;
        std::cout << "1. Find active users:" << std::endl;
        std::cout << "   auto users = entity_manager::instance().query<User>(db)" << std::endl;
        std::cout << "                  .where(\"is_active = true\")" << std::endl;
        std::cout << "                  .order_by(\"username\")" << std::endl;
        std::cout << "                  .execute();" << std::endl;

        std::cout << "\n2. Find products by price range:" << std::endl;
        std::cout << "   auto products = entity_manager::instance().query<Product>(db)" << std::endl;
        std::cout << "                     .where(\"price >= 10.0 AND price <= 100.0\")" << std::endl;
        std::cout << "                     .where(\"is_available = true\")" << std::endl;
        std::cout << "                     .limit(10)" << std::endl;
        std::cout << "                     .execute();" << std::endl;

        std::cout << "\n3. Aggregation queries:" << std::endl;
        std::cout << "   auto count = entity_manager::instance().query<User>(db).count();" << std::endl;
        std::cout << "   auto avg_price = entity_manager::instance().query<Product>(db).avg(\"price\");" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error in query demonstration: " << e.what() << std::endl;
    }
}

void demonstrate_entity_lifecycle() {
    std::cout << "\n=== Entity Lifecycle Demonstration ===" << std::endl;

    // Create new entities
    User user;
    user.username = "demo_user";
    user.email = "demo@example.com";
    user.full_name = "Demo User";

    Product product;
    product.name = "Demo Product";
    product.description = "A sample product for demonstration";
    product.price = 19.99;
    product.stock_quantity = 100;

    std::cout << "Created entities:" << std::endl;
    user.print_info();
    product.print_info();

    // Demonstrate validation
    std::cout << "\nValidation results:" << std::endl;
    std::cout << "User is valid: " << (user.is_valid() ? "Yes" : "No") << std::endl;
    std::cout << "Product is in stock: " << (product.is_in_stock() ? "Yes" : "No") << std::endl;

    // In a real application with database connection:
    std::cout << "\nLifecycle operations (requires database):" << std::endl;
    std::cout << "1. user.save() - Insert/update entity" << std::endl;
    std::cout << "2. user.load() - Load from database by primary key" << std::endl;
    std::cout << "3. user.update() - Update existing record" << std::endl;
    std::cout << "4. user.remove() - Delete from database" << std::endl;
}

int main() {
    std::cout << "=== ORM Framework Demonstration ===" << std::endl;
    std::cout << "This sample demonstrates the C++20 concepts-based ORM framework" << std::endl;
    std::cout << "with type-safe entity definitions and automatic schema management." << std::endl;

    try {
        demonstrate_entity_definition();
        demonstrate_schema_management();
        demonstrate_type_safety();
        demonstrate_entity_queries();
        demonstrate_entity_lifecycle();

        std::cout << "\n=== ORM Framework Features Summary ===" << std::endl;
        std::cout << "✓ C++20 concepts-based entity definitions" << std::endl;
        std::cout << "✓ Compile-time type safety" << std::endl;
        std::cout << "✓ Automatic schema generation" << std::endl;
        std::cout << "✓ Field constraints and metadata" << std::endl;
        std::cout << "✓ Type-safe field accessors" << std::endl;
        std::cout << "✓ Entity lifecycle management" << std::endl;
        std::cout << "✓ Query builder integration" << std::endl;

        std::cout << "\nFor complete functionality, connect to a database and use:" << std::endl;
        std::cout << "  entity_manager::instance().create_tables(db);" << std::endl;
        std::cout << "  auto results = entity_manager::instance().query<EntityType>(db)...execute();" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}