function drawSheet(canvas, parts) {
    if (!parts || parts.length === 0) {
        console.warn("No parts to draw");
        return;
    }

    const ctx = canvas.getContext('2d');
    const scale = 4;

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    parts.forEach(p => {
        const px = p.x * scale;
        const py = p.y * scale;
        const pw = p.w * scale;
        const ph = p.h * scale;

        // Draw the slab
        ctx.fillStyle = "#d2b48c";
        ctx.fillRect(px, py, pw, ph);

        ctx.strokeStyle = "#5d4037";
        ctx.lineWidth = 1.5;
        ctx.strokeRect(px, py, pw, ph);

        // Calculate center for text
        const centerX = px + (pw / 2);
        const centerY = py + (ph / 2);

        ctx.save();
        ctx.fillStyle = "black";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";

        // Logic: If the slab is very narrow (width < 40px) and tall, rotate text
        const shouldRotate = pw < 40 && ph > pw;

        if (shouldRotate) {
            ctx.translate(centerX, centerY);
            ctx.rotate(-Math.PI / 2); // Rotate 90 degrees counter-clockwise
            
            ctx.font = "bold 10px Arial";
            ctx.fillText(p.label, 0, -6);
            
            ctx.font = "9px Arial";
            ctx.fillText(p.dims + " cm", 0, 8);
        } else {
            if (pw > 20 && ph > 15) {
                ctx.font = "bold 11px Arial";
                ctx.fillText(p.label, centerX, centerY - 5);

                ctx.font = "10px Arial";
                ctx.fillText(p.dims + " cm", centerX, centerY + 10);
            }
        }
        ctx.restore();
    });
}