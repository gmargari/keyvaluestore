#!/bin/bash
git diff --staged $* | kompare -
