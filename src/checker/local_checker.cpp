#include "local_checker.h"
#include "../logger/logger.h"
#include <iostream>

void LocalChecker::type_check(std::vector<std::shared_ptr<Stmt>> stmts) {
    for (auto stmt : stmts) {
        try {
            stmt->accept(this);
        } catch (const LocalTypeException&) {
            // Do nothing
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
