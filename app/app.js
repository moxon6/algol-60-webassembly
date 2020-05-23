function setElementProperty(selector, prop, value) {
    const element = document.querySelector(selector);
    
    let obj = element
    const path = prop.split(".")

    while (path.length > 1) obj = obj[path.shift()]
    obj[path[0]] = value;
}

const log = console.log.bind(console)
const CALL_PREFIX = "CALL:"
console.log = (message, ...args) => {
    if (message.startsWith(CALL_PREFIX)) {
        setTimeout(() => eval(message.slice(CALL_PREFIX.length)), 0)
    } else {
        log(message);
    }
        
}