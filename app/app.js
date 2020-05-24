function setElementProperty(selector, prop, value) {
    const element = document.querySelector(selector);
    
    let obj = element
    const path = prop.split(".")

    while (path.length > 1) obj = obj[path.shift()]
    obj[path[0]] = value;
}

const log = console.log.bind(console)

let drawRequests = [];
console.log = (message, ...args) => {
    if (message.startsWith("CALL:")) {
        drawRequests.push( message.slice("CALL:".length).replace(" ", "") )
    } else if (message.startsWith("EXECUTE")) {
        eval(drawRequests.join(";"))
        drawRequests = [];
    } else {
        log(message);
    }
}

const keysDown = {};
window.onkeydown = e => keysDown[e.keyCode] = true;
window.onkeyup = e => keysDown[e.keyCode] = false;

const getKeyDown = () => Object.keys(keysDown).find(key => keysDown[key]);

const inputBuffer = [ Math.floor(Math.random() * 500), null ]; // First input request is for a random seed

window.prompt = () => {    
    if (!inputBuffer.length) {
        inputBuffer.push(getKeyDown() || 0);
        inputBuffer.push(null);
    }
    return inputBuffer.shift();
}
