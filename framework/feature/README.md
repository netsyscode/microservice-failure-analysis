# Features Directory

Welcome to the `features` directory! 
This folder is a crucial part of our project, housing all the individual features that make our application functional and unique. 
Each feature within this directory is organized into its own sub-folder, allowing for modular development and easy navigation.

## Structure

Every sub-folder within the `features` directory represents a separate feature of our application. Here's what a typical feature folder might contain:

- **Src codes**: Core logic and functionality of the feature.
- **Data files**: Any static data files that the feature might use.
- **Tests**: Unit and integration tests specific to this feature.

Here's an example of what the directory structure might look like:

```
features/
├── feature1/
│   ├── src/
│   ├── tests/
│   └── feature1_main.py
├── feature2/
│   ├── src/
│   ├── tests/
│   └── feature2_main.py
└── ...
```

## Dependencies

Many features within this directory rely on shared components located in the `agent` folder, specifically under `agent/kernel` and `agent/user`. 
These components provide essential functionalities like compilation and interaction with the eBPF program, which are leveraged by our features to perform their tasks.

## How to Add a New Feature

To add a new feature to our project, please follow these steps:

1. **Create a new sub-folder** under `features`, named after your feature.
2. **Develop your feature** within this sub-folder. Make sure to include any Python modules, data files, and tests.
3. **Document your feature** by adding a README.md file to your sub-folder, explaining what your feature does and how it works.