# GeminiSketch Experiment Framework

This document describes how to reproduce the experiments of GeminiSketch as specified in the experimental requirements.

## Experiment Overview

The experiment framework is designed to evaluate the performance of GeminiSketch against various baseline methods on temporal graph datasets. The framework measures several key metrics including Average Relative Error (ARE) for different types of queries, precision for reachability queries, and throughput.


## Datasets

The framework uses four real-world temporal graph datasets:

1. **Stackoverflow**: 2,601,977 vertices, 63,497,050 edges
2. **Wiki**: 1,140,149 vertices, 7,833,140 edges
3. **Reddit**: 55,863 vertices, 858,490 edges
4. **Super User**: 194,085 vertices, 1,443,339 edges

The datasets should be placed in the `../Dataset/` directory.


## Compilation

To compile the experiment program, use the provided Makefile:

```bash
make experiment
```

This will compile the experiment program along with all required dependencies.

## Running the Experiments

To run the experiments, execute the compiled program:

```bash
./experiment
```

