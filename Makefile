PY_DIRS := callprofiler tests

FLAKE8_BIN ?= pipenv run flake8
MYPY_BIN ?= pipenv run mypy
NOSETESTS_BIN ?= pipenv run nosetests
PYTHON_BIN ?= pipenv run python

all: install test stylecheck typecheck

build:
	@$(PYTHON_BIN) setup.py build

install:
	@$(PYTHON_BIN) setup.py install

test:
	@$(NOSETESTS_BIN) tests

stylecheck:
	@$(FLAKE8_BIN) '--exclude=.svn,CVS,.bzr,.hg,.git,__pycache__,.tox,.#*' $(PY_DIRS)

typecheck:
	@$(MYPY_BIN) $(PY_DIRS)

clean:
	@find $(PY_DIRS) -type d -name __pycache__ | xargs rm -rf
	@rm -rf .mypy_cache build

dist:
	@$(PYTHON_BIN) setup.py sdist

.PHONY: all test stylecheck clean dist
