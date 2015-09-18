yahdlc_test:
	@$(CXX) -g -O0 -fprofile-arcs -ftest-coverage -Wall -Wextra -Werror -I../ ../yahdlc.c ../fcs16.c yahdlc_test.cpp -o yahdlc_test -lboost_unit_test_framework

test: yahdlc_test
	@./yahdlc_test --log_level=test_suite

coverage: yahdlc_test
	@lcov --directory . --zerocounters -q
	@./yahdlc_test --log_level=test_suite
	@lcov --directory . --capture --output-file all.info -q
	@rm -rf coverage
	@lcov  --remove all.info "yahdlc_test.cpp" "boost/*" "c++/*" -o report.info -q

coveralls: coverage
	@coveralls-lcov -t $(REPO_TOKEN) report.info

coverage-html: coverage
	@genhtml -o coverage report.info

clean:
	@rm -rf yahdlc_test coverage *.g* *.info
