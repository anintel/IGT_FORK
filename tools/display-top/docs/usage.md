# **Usage Guide**  

## **Navigating DisplayTop**  

DisplayTop provides an ncurses-based interface for analyzing display pipeline data. The tool features multiple menus and actions, allowing users to navigate and interact with the system efficiently.  

### **Menu Structure**  

When you launch DisplayTop, you will see the following menu options:  
1. **Display Top** – Overview of the display pipeline.  
2. **Display Configuration** – Shows details about the current display setup.  
3. **Display Debugfs** – Provides access to display-related debugfs entries.  
4. **MMIO Registers** – Displays MMIO register information.  
5. **DPCD Registers** – Lists DisplayPort Configuration Data (DPCD) registers.  
6. **Ftrace** – Displays ftrace logs related to display activity.  
7. **Exit** – Closes the application.  

---

> **Note:**  
> - Running DisplayTop without `sudo` may result in some menu options not being visible.  
> - If the respective JSON files for MMIO and DPCD registers are not present, the corresponding menus will not be available.  
> - The Ftrace menu will not be able to provide register name information without the aforementioned JSON files.

---

To navigate the menu, use:  
- **Arrow keys** – Move up and down between menu items.  
- **Enter** – Select the highlighted menu item.  
- **Mouse or Scrollpad** – Scroll through the options.  

---

### **Available Actions**  

While interacting with DisplayTop, you can perform various actions using keyboard shortcuts:  

| Key | Action |  
|------|-------------------------------------------|  
| **S** | Search within the current menu. |  
| **ESC** | Exit the current menu or application. |  
| **P** | Dump the current page’s data. |  
| **D** | Perform a recursive dump of all data in this node and in its children. |  
| **R** | Refresh the current node. |  

---

### **Scrolling and Mouse Support**  

- You can use the **scrollpad or mouse wheel** to scroll through long entries.  
- Clicking on menu items using the mouse will **NOT** allow selection.  

---

By using these navigation methods and actions, you can efficiently explore and analyze display-related data using DisplayTop.