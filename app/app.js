function setElementProperty(selector, prop, value) {
    const element = document.querySelector(selector);
    
    let obj = element
    const path = prop.split(".")

    while (path.length > 1) obj = obj[path.shift()]
    obj[path[0]] = value;
}

const params = new URLSearchParams(window.location.search)

const log = console.log.bind(console)
const CALL_PREFIX = "CALL:"
console.log = (message, ...args) => {
    if (message.startsWith(CALL_PREFIX)) {
        setTimeout(() => eval(message.slice(CALL_PREFIX.length).replace(" ", "")), 0)
        if (params.get("debug")) {
            log(message);
        }
    } else {
        log(message);
    }
        
}