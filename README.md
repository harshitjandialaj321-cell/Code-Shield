# Code-Shield
Code Shield: Source Code Similarity and Plagiarism Detection System

Code Shield is a project designed to detect similarity between C/C++ source-code submissions. It is mainly focused on helping teachers identify potentially copied or highly similar programming assignments. Instead of comparing programs only as plain text, Code Shield processes the source code and looks at its structure and important parts. It removes unnecessary differences such as comments, extra spaces, and changes in variable names before comparing the programs.

## Features

* Compare multiple C/C++ source-code files
* Remove comments and unnecessary formatting
* Tokenize source code
* Normalize identifiers
* Generate k-grams
* Generate hash values
* Use the Winnowing technique to create fingerprints
* Compare fingerprints between programs
* Calculate a similarity percentage
* Identify matching code regions
* Generate a simple similarity report

## How It Works

The basic workflow of Code Shield is:

Source Code Files
       ↓
Preprocess / Tokenize
       ↓
Normalize Identifiers
       ↓
Generate k-grams
       ↓
Hashing
       ↓
Winnowing / Fingerprints
       ↓
Fingerprint Matching
       ↓
Similarity Score
       ↓
Report / Review

## Project Approach

The system will be developed using C/C++. First, the submitted source-code files are read and basic unnecessary elements such as comments and extra spaces are removed.
The source code is then divided into tokens such as keywords, variables, and operators. Identifiers are normalized so that simple variable-name changes do not completely hide similarity.
After normalization, the tokens are divided into small groups called k-grams. Hash values are generated for these k-grams, and the Winnowing method is used to select important fingerprints.
Finally, fingerprints from different student submissions are compared to calculate a similarity score and identify matching code sections.

## Technologies / Concepts

* C/C++
* Tokenization
* Code Normalization
* k-grams
* Hashing
* Winnowing Algorithm
* Fingerprinting
* Source Code Similarity Detection

## Project Goals

The main goal of Code Shield is to develop a simple and customizable source-code similarity detection system for C/C++ assignments.

The project aims to help instructors quickly identify submissions that may require further checking.

## Expected Outcome

The final system will be able to compare C/C++ programs, calculate their similarity percentage, and highlight code sections where matching is found.

It will also generate a simple report that can help teachers review potentially similar submissions.


## Team

### Team Name

**PlagX**

### Team Members

**Gaurav Dubey** — Team Lead
**Harshit Jandial**

## References

1. S. Schleimer, D. S. Wilkerson and A. Aiken, *Winnowing: Local Algorithms for Document Fingerprinting*, ACM SIGMOD, 2003.
2. Stanford University, *MOSS: A System for Detecting Software Similarity*.
3. Stanford University, Alex Aiken Publications, *Winnowing: Local Algorithms for Document Fingerprinting*.
