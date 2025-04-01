# **DisplayTop**  

A terminal-based tool for monitoring and debugging the display pipeline.

## **Features**  
- Terminal UI using `ncurses`  
- DPCD register read
- Dump feature for all the menus
- MMIO register tracing and parsing
- Real-time display performance analysis  
- Automatic dependency checking and installation  

---

## **Quick Start**  

Clone the repository and just make run
```
git clone https://github.com/anintel/display-top.git
cd display-top
```

To Run the tool:  
```sh
make run
```
`make run` does all the job from checking for dependencies, installing them, building the tool and running it but if you want to do these steps individually 
follow the instructions below.

---

### **Installation**  
For detailed installation steps, refer to [docs/installation.md](docs/installation.md).  

To manually install dependencies:  
```sh
make setup
```

---

### **Building the Project**  
To compile the project:  
```sh
make
```
To clean build artifacts:  
```sh
make clean
```

---

## **Documentation**  
| Section | Description |  
|---------|------------|  
| [Docs Index](docs/index.md) | Detailed Index for Documentation. |  
| [Installation](docs/installation.md) | Setup instructions. |  
| [Usage](docs/usage.md) | How to use DisplayTop. |  
| [How It Works](docs/architecture.md) | Internal workflow and components. |  
| [Troubleshooting](docs/troubleshooting.md) | Common issues and fixes. |  
| [Contributing](docs/contributing.md) | Guidelines for contributors. |  

---

## **Support**  
- Check the [Troubleshooting Guide](docs/troubleshooting.md) for common issues.  
- Open an issue in the repository for further assistance.  

<br>

Built by **Linux Display India @Intel**.  

---