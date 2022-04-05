# Fast Fully Secure Replicated Secret Sharing

Included here is an implementation of core components of the protocols described
in the paper "Fast Fully Secure Multi-Party Computation Over Any Ring with
Two-Thirds Honest Majority".

This application comprises three different experiments, written as separate
compilable executables.

We've strived to make the code somewhat navigable.

## The Code

The code in this repository is written in C++ and targets C++17. support for
AES-NI is required for the PRG to work (see `src/frn/lib/primitives/prg.cc`).

Code directly related to the protocol is located in `src/frn/`, while code in
`src/frn/lib` contain various auxiliary code that was copied from another
private repositry so as ensure the anonymity of this submission.

## Building

Our code has zero external dependencies, which should make building and running
it fairly straightforward.

```
cmake . -B build
cd build
make
```

## Running

The `run.sh` script in the root directory is what was used to generate the data
underlying section 7 in our paper. This script will locally run one of the
experiments for some number of parties and a number of inputs, multiplications
or checks (depending on the executable).

For example, to run the input experiment (see `src/frn/exp_input.cc`) with 7
parties and 1000 inputs, one would run

```
./run.sh build/exp_input.x 7 1000
```

Each party will report both communication and time spent in different steps of
the protocol. These outputs can be found in
`logs/logs_exp_input_7_1000_<timestamp>/party_<i>.log` for the example above.

Available experiments are

* `exp_input.x` performs an experiment where party 0 inputs some provided number
  of inputs

* `exp_mult.x` executes a number of secure multiplications.

* `exp_check.x` executes a number of checks.
