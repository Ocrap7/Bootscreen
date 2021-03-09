// int mfd = open("/dev/input/event4", O_RDONLY | O_NONBLOCK);
// sleep(1);
// while (0)
// {
//     //     for (uint32_t y = start; y < info->yres; y++)
//     //     {
//     //         for (uint32_t x = 0; x < info->xres; x++)
//     //         {
//     //             uint32_t col = interp(RGB(255, 0, 50), RGB(0, 255, 200), x * step);
//     //             pixels[y * info->xres_virtual + x] = col;
//     //         }
//     //     }
//     //     rect(mouseX, mouseY, 5, 5, 0);
//     struct input_event event;
//     int nbytes = read(kfd, &event, sizeof(event));
//     if (nbytes > 0)
//     {
//         if (event.type == EV_KEY && event.value == 1 && event.code == KEY_ESC)
//             break;
//     }
//     //     nbytes = read(mfd, &event, sizeof(event));
//     //     if (nbytes > 0)
//     //     {
//     //         if (event.type == EV_REL)
//     //         {
//     //             switch (event.code)
//     //             {
//     //             case REL_X:
//     //                 mouseX += event.value;
//     //                 break;
//     //             case REL_Y:
//     //                 mouseY += event.value;
//     //                 break;
//     //             }
//     //             if (mouseX >= info->xres)
//     //                 mouseX = info->xres - 1;
//     //             else if (mouseX < 0)
//     //                 mouseX = 0;
//     //             if (mouseY >= info->yres)
//     //                 mouseY = info->yres - 1;
//     //             else if (mouseY < 0)
//     //                 mouseY = 0;
//     //         }
//     //     }
//     //     // usleep(16 * 1000);
// }