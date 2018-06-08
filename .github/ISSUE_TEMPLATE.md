1. Please check that no similar bug is already reported. Have a look on the list of open bugs at https://github.com/anbox/anbox/issues
2. Make sure you are running the latest version of Anbox before reporting an issue. Update snap to latest: `snap refresh --devmode --edge anbox`
3. Make sure you have debug logs enabled:
`sudo snap set anbox debug.enable=true`
4. Reproduce the error while debug logs enabled.
5. Run the anbox logs collection utility and attach the tar file.
`sudo /snap/bin/anbox.collect-bug-info `

6. ** Please paste the result of `anbox system-info` below:**
```
[please paste printout of `anbox system-info` here]
```

**Please describe your problem:**


**What were you expecting?:**


**Additional info:**

